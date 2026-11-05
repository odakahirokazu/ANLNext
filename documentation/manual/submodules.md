# 5. Submodule

[目次](README.md) · [前：モジュール開発](module_development.md) · [次：並列実行](parallel.md)

## Submoduleの役割

Submoduleは正規の解析モジュールの部品です。
パラメータ登録、Evs、同じチェーン内のモジュールへのアクセスを利用できます。
解析アルゴリズムを差し替える用途に限らず、親の処理を構成する任意の部品として使用できます。

チェーンの処理要素にはならず、`mod_analyze()`などをマネージャーから自動で呼ぶ対象ではありません。
親が、部品の公開するメソッドを必要な段階で呼びます。
必要モジュールの存在・位置の宣言である`require_module()`は、正規モジュールだけの機能です。

## C++で実装する

`BasicSubModule`を継承し、`DEFINE_ANL_SUBMODULE`で名前・バージョンと複製処理を定義します。
パラメータの登録先は`define_parameters()`です。
正規モジュールの`ENABLE_PARALLEL_RUN`に相当する複製処理は、このマクロに含まれています。

```cpp
// CalculationAlgorithm.hhの構成例。BasicSubModule.hhをincludeする
class CalculationAlgorithm : public anlnext::BasicSubModule
{
public:
  virtual double calculate(double input) = 0;
};

class MultiplyAlgorithm : public CalculationAlgorithm
{
  DEFINE_ANL_SUBMODULE(MultiplyAlgorithm, 1.0);
public:
  void define_parameters() override;
  double calculate(double input) override;

private:
  double factor_ = 2.0;
};
```

```cpp
// CalculationAlgorithm.ccの定義例
void MultiplyAlgorithm::define_parameters()
{
  define_parameter("factor", &MultiplyAlgorithm::factor_);
}

double MultiplyAlgorithm::calculate(double input)
{
  return input * factor_;
}
```

親は`mod_define()`で名前を宣言します。実体はその後で選択できます。
次は[例題のAlgorithmModule](../../examples/submodule/source/include/AlgorithmModule.hh)に対応する断片です。

```cpp
// 親のメンバ
CalculationAlgorithm* algorithm_ = nullptr;

// 親のmod_define()内
define_submodule("algorithm", &AlgorithmModule::algorithm_);

// 親のset_submodule_by_key(name, key)内でmultiplyを選択した場合
set_submodule(name, std::make_unique<MultiplyAlgorithm>());
```

`set_submodule()`がサービスを接続し、Submoduleの`define_parameters()`を呼んで登録します。
親の`algorithm_`も登録済み実体を指すように設定します。
実体の準備が失敗した場合は例外として設定処理を中断し、トランザクションとしての復旧は行いません。
未知の選択キーに対して何を例外にするかは、親の`set_submodule_by_key()`で実装します。

## 宣言・実体・参照を区別する

| 操作・状態 | 意味 |
| --- | --- |
| `define_submodule(name, member)` | 名前を宣言し、親のポインタメンバへ接続 |
| `define_submodule(name)` | 名前だけを宣言。ユーザーが参照を管理 |
| `set_submodule(name, unique_ptr)` | 宣言済みの名前に実体を登録・交換 |
| `is_submodule_defined(name)` | 名前の宣言の有無を確認 |
| `is_submodule_set(name)` | 宣言済みの名前に実体があるか確認 |
| `get_submodule(name)` | 実体への参照用ポインタを取得 |

未宣言名への設定、同名の二重宣言、nullの登録、未設定実体の取得はエラーです。
`is_submodule_set()`も、名前が未宣言ならエラーになります。
必要な実体の設定を保証してから処理してください。

実体はフレームワークが所有し、親のポインタは参照用です。
親がそのポインタを`delete`したり、同じ実体を別の所有型に引き取ったりしてはいけません。
宣言したメンバの指す型と実体が適合しない場合は`SubModuleBindingError`になります。

同じ名前に再登録すると、実体を交換します。
登録順は維持し、接続したメンバは新しい実体へ更新します。
別途取得していた古い実体へのポインタは無効になるため、保持方法を見直してください。

## スクリプトで選択して設定する

Ruby／Pythonでは、`chain()`の直後に`with_submodule(name, key)`を置き、
その後で`with_parameters()`を呼ぶ順序を基本にします。
選択と設定は予約順に実行されるため、実体を登録してからそのパラメータを設定できます。

```ruby
chain.chain(SubmoduleExample::AlgorithmModule, :processor)
chain.with_submodule('algorithm', 'multiply')
chain.with_parameters(input: 4.0, algorithm_: {factor: 7.0})
```

```python
chain.chain(AlgorithmModule, 'processor')
chain.with_submodule('algorithm', 'multiply')
chain.with_parameters({'input': 4.0, 'algorithm_': {'factor': 7.0}})
```

`with_submodule`は親の選択を保持します。
値の表示・取得は、親から`get_submodule()`で実体を得て、通常のパラメータAPIを使います。
YAMLも値だけを設定するため、実体の選択はスクリプトに残します。

## 複製時の扱い

登録した実体は、親モジュールとともに複製されます。
パラメータ参照先とANLサービスを複製先へ接続し、
`define_submodule(name, member)`で宣言したメンバも、対応する複製実体へ結び直します。
構成は並行チェーン複製前に確定してください。

接続を宣言していないポインタや、Submodule内部の任意の資源は、ユーザーのコピー設計の対象です。
`SubModuleEntry`／`SubModuleMember`は登録・接続を実現する内部用クラスであり、
ユーザーが直接生成する必要はありません。

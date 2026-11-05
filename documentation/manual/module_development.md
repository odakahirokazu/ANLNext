# 4. C++モジュールの開発

[目次](README.md) · [前：パラメータ](parameters.md) · [次：Submodule](submodules.md)

## 最小のモジュール

`BasicModule`を継承し、必要なフックだけoverrideします。
次の例は、入力値に係数を掛け、runごとの合計をresultとして公開します。
ヘッダに宣言、`.cc`に処理の定義を置きます。

`ScaleModule.hh`：

```cpp
#ifndef EXAMPLE_SCALE_MODULE_H
#define EXAMPLE_SCALE_MODULE_H

#include "BasicModule.hh"

class ScaleModule : public anlnext::BasicModule
{
  DEFINE_ANL_MODULE(ScaleModule, 1.0);
  ENABLE_PARALLEL_RUN();
public:
  anlnext::ANLStatus mod_define() override;
  anlnext::ANLStatus mod_begin_run() override;
  anlnext::ANLStatus mod_analyze() override;
  anlnext::ANLStatus mod_merge(const anlnext::BasicModule* module) override;

private:
  double input_ = 4.0;
  double factor_ = 7.0;
  double total_ = 0.0;
};

#endif
```

`ScaleModule.cc`：

```cpp
#include "ScaleModule.hh"

using namespace anlnext;

ANLStatus ScaleModule::mod_define()
{
  define_parameter("input", &ScaleModule::input_);
  define_parameter("factor", &ScaleModule::factor_);
  define_result("total", &ScaleModule::total_);
  return AS_PROCEED;
}

ANLStatus ScaleModule::mod_begin_run()
{
  total_ = 0.0;
  return AS_PROCEED;
}

ANLStatus ScaleModule::mod_analyze()
{
  total_ += input_ * factor_;
  return AS_PROCEED;
}

ANLStatus ScaleModule::mod_merge(const BasicModule* module)
{
  total_ += dynamic_cast<const ScaleModule&>(*module).total_;
  return AS_PROCEED;
}
```

`DEFINE_ANL_MODULE`はクラス名とバージョンを公開します。
`ENABLE_PARALLEL_RUN`はコピーコンストラクタによる複製を有効にします。
この例は値型のメンバだけなので、暗黙生成されるコピーコンストラクタを使用できます。
並列化に必要なことは[第6章](parallel.md)で説明します。

## パラメータを追加する

以下はクラス内のメンバ宣言の断片です。

```cpp
std::vector<int> channels_;
std::map<std::string, double> weights_;
std::uintmax_t address_ = 0;
double duration_ = 0.0;
```

必要な標準ヘッダ`<vector>`・`<map>`・`<string>`・`<cstdint>`を追加し、
`mod_define()`で次のように登録します。

```cpp
define_parameter("channels", &ScaleModule::channels_);
define_parameter("weights", &ScaleModule::weights_);
define_parameter("address", &ScaleModule::address_);
define_parameter("duration", &ScaleModule::duration_, 0.001, "ms");
set_parameter_description("Duration in milliseconds");
```

最後の設定メソッドは、直前に定義・選択したパラメータの説明を設定します。
登録したメンバの値が初期値になります。

レコード型の例は、次の宣言・登録です。

```cpp
// クラス内のメンバ。<tuple>も必要
std::vector<std::tuple<int, std::string, double>> rows_;

// mod_define()内
define_parameter("rows", &ScaleModule::rows_);
add_value_element("count", 0);
add_value_element("label", std::string{});
add_value_element("weight", 1.0);
```

値要素は、tupleの各要素と同じ型・順序で登録します。
ここでは編集用の外部メンバを別に作らず、値を所有する値要素を使っています。

## 他モジュールを使う

名前はmodule_idまたは登録された別名で解決します。
必要なモジュールと位置条件は、`mod_define()`で宣言します。

```cpp
// Consumer::mod_define()内の断片
require_module("producer", anlnext::ModulePosition::before);
```

`before`・`after`は要求先が自分の前・後にあること、`any`は存在することを要求します。
デフォルトは`any`です。全モジュールの定義が終わった段階で検証するため、
宣言時に要求先の名前を即座に解決する必要はありません。
これは配置の検証であり、要求先がoffになったり、SKIPで実行されなかったりすることを防ぐ機構ではありません。

実体は、通常`mod_initialize()`で取得します。
次は`Producer`型が見えているクラスでの断片です。

```cpp
const Producer* producer_ = nullptr;  // クラスのメンバ

// mod_initialize()内
get_module("producer", &producer_);
```

| API | 取得先 | 型の確認 |
| --- | --- | --- |
| `get_module()` | `const T*` | `static_cast`。呼び出し側が型の対応を保証 |
| `get_module_non_const()` | `T*` | 同上。変更可能な参照を明示 |
| `get_module_interface()` | `const T*` | `dynamic_cast`。interfaceとして取得 |
| `get_module_interface_non_const()` | `T*` | 同上。変更可能な参照 |

`get_`系は取得できなければ例外を送出します。
任意の依存先では、対応する`request_module*()`で取得し、`nullptr`を確認できます。
無印のAPIは動的な型検査を行わないため、名前だけで型が保証されるとは考えないでください。
読み書きのアクセス権は、型の確認とは別に検査します。
名前の別名・アクセス権と要求検証の詳細は[モジュール要求](../design/module_requirements_v3.md)を参照してください。

## Evsによる選択条件の集計

Evsは、各反復で成立した条件を名前付きフラグとして記録するサービスです。
モジュールやSubmoduleから利用できます。

```cpp
// mod_define()内
define_evs("accepted");

// mod_analyze()内
if (accepted) {
  set_evs("accepted");
}
```

`accepted`はユーザーが決める条件式です。
フラグは各反復の開始時にリセットされます。
`evs(name)`で同じチェーンの状態を参照できます。
Evsの集計はrunごとにリセットし、並列実行ではフレームワークが集約します。
Submoduleには親と対応するチェーンのサービスを接続します。

## エラーと資源

解析の継続・SKIP・終了はANLステータスで表します。
設定不正など、呼び出し側に理由を伝えて中断する問題には例外を使います。
独自のエラーは`ANLException`を継承した具体的な型で表すと、原因を区別できます。

ファイル、メモリ、ロックなどは、それぞれに適した所有型やRAIIを使って管理します。
コピーで共有すべきか、各チェーンで独立に確保すべきかを明確にし、
独立な資源は複製後の`mod_initialize()`で確保する方法も検討してください。

## Ruby／Pythonへ公開する

C++モジュールは共有ライブラリにし、SWIG interfaceから公開します。
独自の解析モジュールは`BasicModule`を継承していることをSWIGにも知らせます。

Rubyの`scale.i`：

```swig
%module scale
%{
#include "ScaleModule.hh"
%}
%import(module="anlnext/ANL") "ANL.i"
class ScaleModule : public anlnext::BasicModule {};
```

Pythonの`scalepy.i`：

```swig
%module scalepy
%{
#include "ScaleModule.hh"
%}
%import(module="anlnext.anlnextpy") "anlnextpy.i"
class ScaleModule : public anlnext::BasicModule {};
```

`%{ ... %}`は生成されたC++にヘッダを読み込ませ、`%import`は基底型の既存の定義を参照します。
ユーザーが呼ぶ独自メソッドも公開したければ、interfaceにその宣言を追加します。
Rubyでは`Scale::ScaleModule`、Pythonでは`from scalepy import ScaleModule`で利用します。

ビルドには`ANLNextSwigRuby.cmake`／`ANLNextSwigPython.cmake`の補助関数を使えます。
ヘッダ・ライブラリ・SWIG interfaceの探索先を含む完全なCMake例は、
[Submodule例題のCMakeLists.txt](../../examples/submodule/source/CMakeLists.txt)を参照してください。
フレームワークや依存ライブラリのビルド条件を変えた場合は、利用側のC++ライブラリと両バインディングも再ビルドします。

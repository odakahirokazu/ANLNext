# 2. 解析のライフサイクル

[目次](README.md) · [前：導入](getting_started.md) · [次：パラメータ](parameters.md)

## 処理の段階

| マネージャーのAPI | モジュールのフック | 役割 |
| --- | --- | --- |
| `define_analysis()` | `mod_define()` | パラメータ、result、Submodule名、必要モジュールの宣言 |
| `pre_initialize_analysis()` | `mod_pre_initialize()` | 元チェーンの初期化。その後、このAPI内で並行チェーンを複製 |
| `initialize_analysis()` | `mod_initialize()` | 複製後、各チェーンで初期化 |
| `run_analysis(n, console)` | `mod_begin_run()` → `mod_analyze()`の反復 → `mod_end_run()` | 1回のrun |
| run終盤 | `mod_reduce()`／`mod_merge()` | 並列モジュールの結果を元チェーンへ集約 |
| `finalize_analysis()` | `mod_finalize()` | 全run終了後の後処理 |

正常時の流れは次のようになります。

```text
定義 → 設定の適用 → 複製前の初期化 → チェーン複製 → 複製後の初期化
                                                        ↓
                            begin_run → loop → end_run → 集約・集計
                                ↑                           ↓
                                └────── 次のrun ────────────┘
                                                            ↓
                                                         finalize
```

複製前の値を全チェーンへ引き継ぎたい場合は、複製前に設定します。
チェーンごとに確保するファイル・装置などの資源は、通常`mod_initialize()`で用意します。
途中でステータスエラーや例外が生じた場合、正常時の残りのフックがすべて呼ばれるとは限りません。
資源の解放は、メンバのRAIIやデストラクタも含めて設計してください。

## AnalysisChain.run()と設定予約

Ruby／Pythonの`AnalysisChain.run(n)`は、定義からfinalizeまでを一括して実行します。
初期化済みチェーンを再利用するAPIではなく、同じChainで`run()`を繰り返すことはできません。

通常の設定処理は次の順序です。

1. `chain()`でモジュールを並べ、設定操作を予約する。
2. 定義を行い、予約したSubmodule選択とパラメータ設定を適用する。
3. `pre_initialize_analysis()`を呼び、並行チェーンを複製する。
4. `modify`のブロック／関数があれば実行し、その中で追加された設定予約を適用する。
5. `initialize_analysis()`、`run_analysis()`、`finalize_analysis()`を呼ぶ。

定義と設定だけを先に行うこともできます。

```ruby
# chainの構成・設定予約を済ませた後の断片
chain.define
chain.load_all_parameters
chain.get_module(:processor).print_parameters
chain.run(10)
```

```python
# chainの構成・設定予約を済ませた後の断片
assert chain.define() == anlnext.ANLStatus_proceed
chain.load_all_parameters()
chain.get_module('processor').print_parameters()
chain.run(10)
```

`define()`はRubyではマネージャー、PythonではANLステータスを返します。
定義済みなら、`run()`は同じマネージャーを使い、定義を繰り返しません。
表示・値取得は予約の実行を兼ねないため、読み取る前に明示的に適用します。

## 複数回runする

同じ初期化状態で複数回runする場合は、`ANLManager`または`ANLManagerMT`を直接使います。
次は、[第1章の例題](getting_started.md)を使うPythonの完全な実行例です。

```python
import anlnext
from submoduleExamplePy import AlgorithmModule

module = AlgorithmModule()
module.set_module_id('processor')
manager = anlnext.ANLManagerMT(3)
manager.set_modules([module])

def check(code):
    if code != anlnext.ANLStatus_proceed:
        raise RuntimeError(anlnext.status_to_string(code))

check(manager.define_analysis())
module.set_submodule_by_key('algorithm', 'multiply')
module.set_parameter('input', 4.0)
module.get_submodule('algorithm').set_parameter('factor', 7.0)
check(manager.pre_initialize_analysis())
check(manager.initialize_analysis())

for count in [10, 5]:
    check(manager.run_analysis(count, False))
    print(manager.run_index(), anlnext.get_value(module.get_parameter('total')))

check(manager.finalize_analysis())
```

run番号と合計値は、順に`0, 280.0`と`1, 140.0`になります。
この例題は`mod_begin_run()`で合計値をゼロにするため、run間で累積しません。

毎runの開始時に、フレームワークは処理回数・エラーカウント・Evs集計・制御状態をリセットします。
一方、モジュールのメンバや解析結果をリセットするかどうかはユーザーの設計です。
並列結果を累積する場合は、前回集約済みの値を再び加算しないよう、各コピーの値の意味も決めてください。
criticalステータスや例外の後に復旧して再runすることは、今回のサポート範囲に含めません。

## run番号とloop番号

`ANLManager::run_index()`とモジュールの`get_run_index()`は、最初のrunを`0`と数えます。
各runで`get_loop_index()`は`0`から始まる処理対象の反復番号を返します。
`run_analysis(n)`の`n`は試行回数の上限であり、成功した処理数の保証ではありません。
SKIPも1回に数えます。並列実行では番号の割り当て順と処理の完了順が一致するとは限りません。

番号のC++型は`std::size_t`です。
開始前などの未設定値は`undefined_run_index`／`undefined_loop_index`であり、型の最大値を使います。
有効な番号として扱わないでください。

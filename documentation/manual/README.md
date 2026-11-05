# ANLNext 3 マニュアル

ANLNextは、小さな処理を担当するC++モジュールをチェーンとして組み合わせ、
同じ処理を繰り返すデータ処理・解析フレームワークです。
RubyまたはPythonでモジュールを選び、パラメータと実行条件を設定します。
処理対象を物理的なイベントに限定せず、反復の単位をloopと呼びます。

このマニュアルは、version 3の現在の実装を対象にしています。
3.0の正式リリースを宣言するものではありません。
最初に例題を動かしたい場合は第1章、モジュールを開発する場合は第4章から読めます。

## 目次

| 章 | 内容 |
| --- | --- |
| [1. 導入と最初の解析](getting_started.md) | ビルド、環境設定、Ruby／Pythonの実行例 |
| [2. 解析のライフサイクル](lifecycle.md) | 定義、初期化、run、終了、複数回run |
| [3. パラメータとYAML](parameters.md) | 型、設定予約、表示・取得、レコード、YAML |
| [4. C++モジュールの開発](module_development.md) | 最小実装、モジュール間アクセス、Evs、バインディング |
| [5. Submodule](submodules.md) | 部品の定義、選択、設定、所有権と複製 |
| [6. 並列実行](parallel.md) | チェーン複製、順序保証、集約、共有データ |
| [7. ステータス・移行・問題の切り分け](reference.md) | 制御コード、v2からの移行、よくある問題 |

## 全体像

```text
Ruby / Python の実行スクリプト
  └─ AnalysisChain：モジュールの選択と設定予約、解析全体の実行
       └─ ANLManager / ANLManagerMT：ライフサイクルとループの管理
            └─ BasicModule のチェーン：順番に処理を実行
                 └─ BasicSubModule：親が必要に応じて利用する部品
```

| 要素 | 役割 |
| --- | --- |
| `BasicModule` | 解析の各段階で呼び出される正規のモジュールの基底クラス |
| `BasicSubModule` | パラメータ登録とANLサービスを利用できる部品の基底クラス |
| `ParameterRegistry` | パラメータとresultの登録・設定・参照 |
| `EvsInterface` | 選択条件のフラグと集計への接続 |
| `ModuleAccessInterface` | 同じチェーン内の他モジュールへのアクセス |
| `ANLManager` | 単一チェーンの管理 |
| `ANLManagerMT` | 複製したチェーンによる並列処理と集約 |
| SWIGバインディング | C++ライブラリとRuby／Pythonの接続 |

フレームワークが処理の段階、登録情報の整合性、サービスへの接続を管理し、
各モジュールが解析アルゴリズムとデータの意味を決めます。
Submoduleをいつ、どのように使うかも、親モジュールの設計に委ねられています。

## マニュアルとほかの資料

このマニュアルは利用方法を順に説明します。
詳細な変更理由や内部実装は、[ループ制御](../design/loop_control_v3.md)、
[パラメータ管理](../design/parameter_management_v3.md)、[APIの命名と移行](../design/naming_v3.md)、
[モジュール要求](../design/module_requirements_v3.md)、[YAML仕様](../design/yaml_parameters_v3.md)を参照してください。

動作するソース一式は[Submodule例題](../../examples/submodule/README.md)にあります。
C++の個々の宣言は[source/include](../../source/include)、
バインディングの補助APIは[Ruby](../../source/bindings/ruby/library/anlnext)／
[Python](../../source/bindings/python/library/anlnext)が参照元です。
既存の[Doxygen設定](../doxygen/Doxyfile)ではC++ APIリファレンスを生成できます。

コマンド例のパスは、自分のインストール先に合わせて変更してください。
「断片」と明記したコードは、該当クラスやスクリプトの中へ組み込んで使います。

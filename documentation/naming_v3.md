# Version 3のメソッド・変数名

独自のメソッド名と変数名をsnake_caseに統一します。
クラス名・型名、マクロ・ステータス定数、ROOTやMPIなど外部ライブラリのAPI名は
それぞれの命名規則に従います。

## 解析ライフサイクル

C++、Ruby、Pythonで、ANLManagerの公開APIを同じ名前に揃えます。

| 旧名 | 新名 |
|---|---|
| `Define()` | `define_analysis()` |
| `PreInitialize()` | `pre_initialize_analysis()` |
| `Initialize()` | `initialize_analysis()` |
| `Analyze()` | `run_analysis()` |
| `Finalize()` | `finalize_analysis()` |

Rubyの`initialize`はコンストラクタに使われるため、解析の初期化は
`initialize_analysis()`と明示します。`run_analysis()`は解析実行の公開APIで、
既存の内部メソッド`process_analysis()`とは別の役割です。
モジュールの`mod_define()`、`mod_initialize()`などのフック名はそのまま使います。
Ruby／PythonのAnalysisChainが提供する`define()`と`run()`もそのままです。

ANLExceptionの`SetVerboseLevel()`は`set_verbose_level()`、
`VerboseLevel()`は`verbose_level()`に変更します。
上記の旧名は新名に置き換えてください。

## ModuleAccess

| 旧名 | 新名 |
|---|---|
| `get_module_NC()` | `get_module_non_const()` |
| `get_module_IF()` | `get_module_interface()` |
| `get_module_IFNC()` | `get_module_interface_non_const()` |
| `request_module_NC()` | `request_module_non_const()` |
| `request_module_IF()` | `request_module_interface()` |
| `request_module_IFNC()` | `request_module_interface_non_const()` |

標準の`get_module()`は`const T*`を取得します。
変更可能な`T*`を取得する場合は、明示的に`_non_const`を付けます。
`_interface`はinterfaceとして`dynamic_cast`で取得することを表し、
無印は従来どおり`static_cast`を使います。読み取り・書き込みのアクセス権の区別も維持します。
`request_module()`系も同じ命名規則です。

移行用に旧名をdeprecatedな転送メソッドとして残します。

## 内部実装・補助コード・サンプル

`module_ID_`、`copy_ID_`などのメンバを`module_id_`、`copy_id_`へ統一します。
`copy_id()`の名前と採番方式は維持します。
ローカル変数、非型テンプレート引数もsnake_caseに統一します。

RubyのSWIGModule補助クラスでは、`includeFiles`、`importModules`、`includeModules`を
それぞれ`include_files`、`import_modules`、`include_modules`へ変更します。

サンプルの独自メソッドも`GetDirectory()`から`get_directory()`、
`FilenameBase()`から`filename_base()`などへ改名します。
パラメータ登録名やモジュールIDなどの設定データは維持します。

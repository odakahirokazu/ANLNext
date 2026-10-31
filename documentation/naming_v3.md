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

## Pythonのrecord入力

record_map／record_vectorへ直接入力するAPIは、Pythonの補助関数へ移します。

```python
# 旧API
module.insert_to_map('table', 'row', {'count': 7})
module.push_to_vector('rows', {'count': 7})

# 新API
anlnext.insert_to_map(module, 'table', 'row', {'count': 7})
anlnext.push_to_vector(module, 'rows', {'count': 7})
```

AnalysisChainの`chain.insert_to_map()`／`chain.push_to_vector()`は変更しません。
辞書の確認と走査はPython側で行い、個々の値の型変換・数値範囲確認は
既存のSWIG setterとC++の型チェックに任せます。
辞書のキーは値要素名の文字列、値は`bool`・`int`・`float`・`str`を使います。
不正な入力は例外となり、レコードを追加しません。要素を省略した場合や空の辞書では
編集用バッファの現在値を使います。通常の挿入後はバッファが初期値へ戻ります。

## Pythonのvector設定

AnalysisChainのリスト設定は、定義後のパラメータ型からsetterを選びます。
`with_parameters({'values': []})`はvectorを空にします。
先頭要素の型による推測は行わず、要素の変換には既存のSWIG APIを使います。
直接操作する場合も、`anlnext.set_vector_parameter(module, 'values', [])`を利用できます。
対象がvectorでない場合や、要素の型・数値範囲が不正な場合は例外になります。

## 内部実装・補助コード・サンプル

`module_ID_`、`copy_ID_`などのメンバを`module_id_`、`copy_id_`へ統一します。
`copy_id()`の名前と採番方式は維持します。
ローカル変数、非型テンプレート引数もsnake_caseに統一します。

RubyのSWIGModule補助クラスでは、`includeFiles`、`importModules`、`includeModules`を
それぞれ`include_files`、`import_modules`、`include_modules`へ変更します。

サンプルの独自メソッドも`GetDirectory()`から`get_directory()`、
`FilenameBase()`から`filename_base()`などへ改名します。
パラメータ登録名やモジュールIDなどの設定データは維持します。

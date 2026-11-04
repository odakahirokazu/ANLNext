# Submoduleのbinding経由での利用例

`SubmoduleExample`は`algorithm`という名前を宣言し、`multiply`という
キーワードで`ExampleAlgorithm`を登録します。アルゴリズムは入力値に
パラメータ`factor`を掛け、親はその結果を`result`に保存します。

Rubyでは既存の`with_parameters`ブロックを使います。

```ruby
chain.chain(Submodule_ruby::SubmoduleExample, :example)
chain.with_parameters do |mod|
  mod.set_submodule_by_key('algorithm', 'multiply')
end
chain.with_parameters("algorithm.factor": 7)
chain.with_parameters(input: 4)
```

Pythonでは既存の`with_setter`を使います。

```python
def configure(module):
    module.set_submodule_by_key('algorithm', 'multiply')

chain.chain(SubmoduleExample, 'example').with_setter(configure)
chain.with_parameters({'algorithm.factor': 7})
chain.with_parameters({'input': 4})
```

これらの設定処理は`define_analysis()`の後、`pre_initialize_analysis()`の前に実行されます。
実体登録時にSubmoduleのパラメータが定義されるため、続けて値を設定できます。
実体を差し替えた場合は古い実体への参照を使わず、再取得します。

`submodule.rb`と`submodule.py`は上記の利用例を拡張した統合テストです。
3チェーンで、パラメータの独立性、各チェーンへのModuleAccess・Evs接続、
再登録時の初期値、未設定の取得や複製後の再登録の例外を確認します。
テスト内の`modify`は複製後のパラメータ値だけを変更し、構成は変更しません。

通常のビルド後、以下で実行できます。

```sh
ctest --test-dir <build-directory> -R 'submodule_(python|ruby)' --output-on-failure
```

bindingと補助ライブラリはビルドディレクトリ内で読み込みます。
インストール済みのANLNextは変更しません。

利用者向けの例題は[`examples/submodule`](../../examples/submodule/README.md)にあります。
掛け算と加算の選択、パラメータ設定、表示・取得、並列実行を扱います。

`with_submodule(name, key)`で実体を選択し、
`with_parameters()`の`"algorithm.factor"`のようなキーで値を設定できます。
親とSubmoduleは値設定処理を共用します。
Rubyでは定義前は設定を予約し、定義後は直ちに適用します。
Pythonでは既存の`with_parameters()`と同じく`load_all_parameters()`時に適用します。
適用時に親から実体を取得するため、それより前に実体を登録しておく必要があります。
未宣言・未設定のSubmoduleや未知のパラメータへの設定は、既存の例外になります。

## パラメータ出力

親のJSONオブジェクトには、Submoduleの宣言がある場合だけ`submodule_list`を
追加します。C++のJSON出力とRubyの`parameters_to_object()`は同じ構造です。

- `registered_name`: 親への登録名。
- `state`: 実体があれば`set`、名前だけの宣言なら`unset`。
- `name`、`version`、`parameter_list`: 実体がある場合だけ出力。

実体名は選択キーワードではありません。この出力は設定状態の記録であり、
キーワードを復元して実体を再構成する機能は含みません。
数値などのJSON表現は従来どおりで、C++のBoost.PropertyTree出力は文字列、
Rubyの出力はRubyの値の型に従います。

画面表示では、親のパラメータに続けて登録名と実体名・バージョンを見出しに
Submoduleのパラメータを表示します。未設定の宣言は`[optional: unset]`のように
表示します。JSONではresultパラメータも記録し、画面のパラメータ表示では
従来どおりresultを除外します。

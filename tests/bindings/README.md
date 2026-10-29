# Submoduleのbinding経由での利用例

`SubmoduleExample`は`algorithm`という名前を宣言し、`multiply`という
キーワードで`ExampleAlgorithm`を登録します。アルゴリズムは入力値に
パラメータ`factor`を掛け、親はその結果を`result`に保存します。

Rubyでは既存の`with_parameters`ブロックを使います。

```ruby
chain.chain(Submodule_ruby::SubmoduleExample, :example)
chain.with_parameters do |mod|
  mod.set_submodule_by_key('algorithm', 'multiply')
  mod.get_submodule('algorithm').set_parameter_integer('factor', 7)
end
chain.with_parameters(input: 4)
```

Pythonでは既存の`with_setter`を使います。

```python
def configure(module):
    module.set_submodule_by_key('algorithm', 'multiply')
    module.get_submodule('algorithm').set_parameter_integer('factor', 7)

chain.chain(SubmoduleExample, 'example').with_setter(configure)
chain.with_parameters({'input': 4})
```

これらの設定処理は`Define()`の後、`PreInitialize()`の前に実行されます。
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

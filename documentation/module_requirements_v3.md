# モジュールの存在・位置の要求

モジュールは`mod_define()`で、必要なモジュールとチェーン内の位置を宣言できます。
`require_module()`は`BasicModule`のprotectedメソッドです。

```cpp
ANLStatus MyModule::mod_define()
{
  require_module("GenerateEvents", ModulePosition::before);
  require_module("SaveData", ModulePosition::after);
  require_module("Calibration");
  return AS_OK;
}
```

`ModulePosition`は`anlnext`名前空間のenum classです。

| 指定 | 条件 |
|---|---|
| `before` | 要求先が自分より前にある |
| `after` | 要求先が自分より後にある |
| `any`（デフォルト） | 要求先がチェーン内に存在する |

`before`と`after`では自分自身を認めません。`any`では自分自身も認めます。
複数回宣言すると、すべての条件を要求します。
この機構は配置順序を確認します。イベントごとのSKIP、オン・オフなどによる
実行の有無や、処理結果までは判定しません。

## 宣言と検証のタイミング

`require_module()`の呼び出しは条件を記録します。その場で名前を解決しないので、
後続モジュールが`mod_define()`で設定するIDや別名も指定できます。

通常のモジュールの要求は、全モジュールの定義と名前・別名の登録が終わった
`ANLManager::Define()`の最後に検証します。
要求は正規のモジュールの`mod_define()`で宣言してください。

存在しないモジュールや位置条件の違反では`ModuleRequirementError`を投げます。
例外には要求元のモジュールID・クラス名、要求先の名前、位置条件、理由が付きます。
Ruby／Pythonからの`Define()`や並行チェーン構築中の違反はRuntimeErrorとして伝わります。

## 名前解決とアクセス権

モジュールIDと別名は、既存のModuleAccessの登録結果に従って解決します。
別名の競合で`yield`・`overwrite`・`remove`を使った場合も、最終的に解決される
実体の位置で判定します。

存在・位置の確認では読み書きのアクセス権を要求しません。
`no_access`のモジュールでも、名前が登録されていれば条件を満たせます。
データを取得する場合は、従来どおり`get_module()`などがアクセス権を確認します。
`privacy`のモジュールは名前自体が登録されないため、要求先として解決できません。
要求元が`privacy`である場合は、そのチェーン内の位置を使って検証できます。

## 並行チェーン

この機能は正規のモジュールだけが持ち、Submoduleではサポートしません。

`duplicate()`は要求を値としてコピーします。
複製されたチェーンは、自身のモジュール実体に対して名前と順序を登録し、
要求を再検証します。元チェーンのオブジェクトへの参照は要求情報に保持しません。

```sh
ctest --test-dir <build-directory> -R module_requirements --output-on-failure
```

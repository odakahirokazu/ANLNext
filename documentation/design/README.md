# version 3の設計・仕様資料

このディレクトリには、仕様と設計判断、変更点を扱う資料をまとめます。
利用方法を順に読む場合は、[日本語マニュアル](../manual/README.md)を参照してください。

| 資料 | 内容 |
| --- | --- |
| [ループ制御](loop_control_v3.md) | REDO廃止、OrderKeeper、終了処理、複数run、ステータス |
| [パラメータ管理](parameter_management_v3.md) | 登録・参照・出力、Submodule、設定予約と値の保持 |
| [モジュール要求](module_requirements_v3.md) | 必要モジュールの存在・配置と検証の時点 |
| [APIの命名と移行](naming_v3.md) | snake_case、ModuleAccess、バインディングのAPI変更 |
| [YAML仕様](yaml_parameters_v3.md) | 入力形式、階層表現、適用順序とエラー処理 |

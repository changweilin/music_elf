# 專案檔案分類

## 正式專案區

- `src/`: C++ 核心實作。
- `include/music_elf/`: 對外標頭與 C API。
- `tools/`: 會被編譯的命令列工具入口，目前是 `music_elf_cli.cpp`。
- `scripts/`: 開發輔助腳本，目前是靜態 dev server。
- `tests/`: C++ 單元測試、整合測試、CLI 測試與真實音訊回歸測試。
- `docs/`: 專案筆記、roadmap、規格與 UI 原型文件。
- `docs/conductor-studio/`: 瀏覽器 UI 原型，透過 `npm run dev` 啟動。
- `agents/`: 本機 agent 與 skill 設定。
- `data/acapella/fixtures/`: CMake 測試會讀取的 WAV fixture。

## 可視為暫存或本機產物

以下路徑不屬於必要原始碼，可由安裝、建置或測試重新產生：

- `build/`
- `build_ninja/`
- `node_modules/`
- `.npm-cache/`
- `.uv-cache/`
- `.uv-python/`
- `data/acapella/processed/`
- `data/acapella/source_media/`
- `build_ninja/Testing/Temporary/`
- `build_ninja/acapella_outputs/`
- `build_ninja/acapella_pipeline_test_outputs*/`

若需要在本機跑真實音訊回歸測試，請保留 `data/acapella/fixtures/` 內的 WAV 檔。

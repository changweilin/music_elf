# Music Elf

> 以 C++17 撰寫的原生音樂分析核心，將單聲道歌唱錄音轉換為可編輯的樂譜、MIDI 與伴奏。

## 專案簡介

**Music Elf** 是一套專注於「歌聲到樂譜」轉換的原生 C++ 核心程式庫。它接收單聲道浮點數 PCM 音訊輸入，依序執行即時音高偵測、音符切分、節奏量化、力度分析、調性辨識與和弦進行推斷，最終輸出可在 DAW 與譜面編輯軟體中直接使用的 **Standard MIDI** 與 **MusicXML 主旋律譜（Lead Sheet）**，並可進一步產生多種風格的伴奏聲部。

本專案以**確定性、可測試、無外部相依**為核心設計原則，所有分析流程皆為純 C++17 演算法，並透過 C ABI 對外公開呼叫介面，方便整合至跨平台應用、外掛或前端 UI。

---

## 核心功能特性

- 🎤 **即時音高偵測**：基於 YIN 演算法的串流式單聲道音高擷取，回報音高、最接近 MIDI 音符、音分偏差與信心度。
- 🎵 **音符切分與量化**：將音高軌跡轉換為帶起訖時間、音高與力度資訊的 `NoteEvent`，並過濾不穩定的瞬態片段。
- 🥁 **節奏與力度分析**：自動估算 BPM 與拍點網格，量化音符的開始時間與時值；同步計算每個音符的 RMS、峰值、MIDI velocity 與動態標記。
- 🎼 **調性與和聲分析**：偵測樂曲調性，並產生多組基於不同風格的候選和弦進行。
- 🎹 **伴奏自動產生**：內建 `block`、`arpeggio`、`broken`、`pad` 等多種伴奏型態，支援轉位、音域限制與簡易聲部進行。
- 📝 **歌詞對齊**：以確定性演算法將已知歌詞 token 對齊至擷取出的音符（非自動語音辨識）。
- 📤 **多格式輸出**：可匯出 Standard MIDI、量化後的 MusicXML 主旋律譜（含小節、休止符、連結線、歌詞、和弦符號、調號、拍號與高音譜記號）。
- 🔊 **內建預覽渲染器**：以簡易振盪器產生 WAV 預覽，無須 SoundFont 或外部音色庫即可快速驗收結果。
- 📚 **General MIDI 和弦資料庫**：批次產生跨樂器、跨根音、跨和弦類型的 `.mid` 範例檔。
- 🔌 **C ABI 對外介面**：透過 `include/music_elf/c_api.h` 暴露完整 C 介面，方便外部語言與 UI 框架整合。

---

## 系統需求與安裝步驟

### 系統需求

| 項目 | 版本 / 說明 |
| --- | --- |
| 編譯器 | 支援 C++17 的編譯器（MSVC 19.20+ / GCC 9+ / Clang 10+） |
| 建置工具 | CMake 3.20 或更高版本 |
| 測試框架 | CTest（隨 CMake 一同安裝） |
| 推薦環境 (Windows) | Visual Studio Community 2022（內含 MSVC 與 CMake） |
| 外部相依 | 無，全部以標準函式庫實作 |

### 取得原始碼

```powershell
git clone <repository-url> music_elf
cd music_elf
```

### Windows（Visual Studio 2022 / PowerShell）

下列指令會依序執行 **設定（configure）→ 建置（build）→ 測試（test）**：

```powershell
cmd /c "call ""C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"" && ""C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"" -S . -B build -G ""Visual Studio 17 2022"" -A x64"
cmd /c "call ""C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"" && ""C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"" --build build --config Release"
cmd /c "call ""C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"" && cd build && ""C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\ctest.exe"" -C Release --output-on-failure"
```

建置完成後，CLI 執行檔位於 `build\Release\music_elf_cli.exe`。

### Linux / macOS

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j
ctest --test-dir build --output-on-failure
```

CLI 執行檔位於 `build/music_elf_cli`。

---

## 快速上手與使用範例

下列範例皆以 Windows PowerShell 路徑為例；Linux/macOS 請將 `build\Release\music_elf_cli.exe` 替換為 `./build/music_elf_cli`。

### 1. 從歌唱錄音產生 MIDI 與 MusicXML

```powershell
build\Release\music_elf_cli.exe input.wav `
    --out-midi output.mid `
    --out-musicxml output.musicxml `
    --lyrics "I can sing" `
    --pattern arpeggio
```

支援的 `--pattern` 伴奏類型：`block`、`arpeggio`、`broken`、`pad`。

### 2. 檢視 WAV 分析摘要（不輸出樂譜檔）

```powershell
build\Release\music_elf_cli.exe inspect input.wav `
    --out-summary inspect.txt `
    --lyrics "I can sing" `
    --pattern block
```

### 3. 將辨識結果渲染為預覽 WAV

```powershell
build\Release\music_elf_cli.exe render-preview input.wav `
    --out-wav preview.wav `
    --waveform triangle `
    --pattern arpeggio
```

### 4. 產生 General MIDI 和弦資料庫

```powershell
build\Release\music_elf_cli.exe generate-catalog `
    --out-dir generated\midi_catalog `
    --instruments all --roots all --chords all
```

若僅需特定子集：

```powershell
build\Release\music_elf_cli.exe generate-catalog `
    --out-dir generated\midi_catalog `
    --instruments 0,40 --roots C,D --chords major,minor,dom7
```

### 5. 快速預覽單一和弦的內建合成音色

```powershell
build\Release\music_elf_cli.exe render-demo `
    --out-wav preview.wav `
    --program 0 --root C --chord major
```

### 6. 量測管線執行效能

```powershell
build\Release\music_elf_cli.exe benchmark input.wav `
    --iterations 5 --out-summary benchmark.txt
```

輸出包含確定性的管線統計值以及 `benchmark_average_ms`、`benchmark_min_ms`、`benchmark_max_ms` 三項實測時間。

### 核心管線流程

```text
mono float32 PCM
  → 即時音高偵測 (YIN)
  → 音符切分 (NoteSegmenter)
  → 節奏量化 + 音符力度分析
  → 調性偵測 + 和弦進行候選
  → 伴奏聲部生成
  → 量化 MIDI / MusicXML 主旋律譜輸出
```

完整演算法狀態（已完成 / 部分完成 / 尚未實作）請參考 `docs/algorithm_flow_status.md`。

---

## 專案架構說明

```text
music_elf/
├── CMakeLists.txt                  # CMake 主設定檔，定義 core 程式庫、CLI、測試目標
├── README.md                       # 本文件
├── .clang-format                   # 程式碼格式設定
├── include/
│   └── music_elf/                  # 公開標頭檔（C++ 與 C ABI 介面）
│       ├── pitch_detector.hpp      #   YIN 音高偵測器
│       ├── note_segmenter.hpp      #   音符切分
│       ├── rhythm_analyzer.hpp     #   BPM / 拍點 / 節奏量化
│       ├── dynamics_analyzer.hpp   #   RMS、velocity、力度標記
│       ├── harmony_analyzer.hpp    #   調性與和弦進行
│       ├── accompaniment_generator.hpp  # 伴奏型態生成
│       ├── lyric_aligner.hpp       #   歌詞 token 對齊
│       ├── midi_writer.hpp         #   Standard MIDI 匯出
│       ├── musicxml_writer.hpp     #   MusicXML 主旋律譜匯出
│       ├── midi_catalog.hpp        #   GM 和弦資料庫產生器
│       ├── audio_io.hpp            #   WAV 讀寫與單聲道降混
│       ├── audio_renderer.hpp      #   內建振盪器預覽渲染
│       ├── core_pipeline.hpp       #   端對端管線整合
│       ├── model_interfaces.hpp    #   模型導向擴充介面定義
│       └── c_api.h                 #   C ABI 對外介面
├── src/                            # 對應上述標頭檔的實作檔
├── tools/
│   └── music_elf_cli.cpp           # CLI 主程式（inspect / generate-catalog / benchmark 等子命令）
├── tests/                          # CTest 單元與整合測試
│   ├── pitch_detector_tests.cpp
│   ├── note_segmenter_tests.cpp
│   ├── rhythm_dynamics_tests.cpp
│   ├── harmony_tests.cpp
│   ├── arrangement_midi_tests.cpp
│   ├── audio_renderer_tests.cpp
│   ├── lyrics_musicxml_tests.cpp
│   ├── audio_io_tests.cpp
│   ├── pipeline_integration_tests.cpp
│   ├── cli_end_to_end_tests.cpp
│   ├── c_api_tests.cpp
│   ├── model_interfaces_tests.cpp
│   ├── midi_catalog_tests.cpp
│   └── export_snapshot_tests.cpp
├── docs/                           # 演算法狀態、模型整合 schema、可行性分析等技術文件
│   ├── algorithm_flow_status.md
│   ├── model_integration_schemas.md
│   └── pre_ui_model_roadmap.md
├── agents/                         # 子代理（subagents）與技能（skills）相關設定
└── build/                          # CMake 建置輸出目錄（git 已忽略）
```

### 主要模組職責一覽

| 模組 | 標頭檔 | 說明 |
| --- | --- | --- |
| 音高偵測 | `pitch_detector.hpp` | 串流式 YIN 演算法，輸出每個分析窗的音高估計 |
| 音符切分 | `note_segmenter.hpp` | 將連續音高估計合併為帶有起訖時間的 `NoteEvent` |
| 節奏分析 | `rhythm_analyzer.hpp` | BPM 估計、拍點對齊、起訖時間量化 |
| 力度分析 | `dynamics_analyzer.hpp` | 計算每個音符的 RMS / 峰值 / velocity / 動態標記 |
| 和聲分析 | `harmony_analyzer.hpp` | 調性偵測與多種風格的候選和弦進行 |
| 伴奏生成 | `accompaniment_generator.hpp` | block / arpeggio / broken / pad 等伴奏型態 |
| 歌詞對齊 | `lyric_aligner.hpp` | 確定性歌詞 token 對齊（非 ASR） |
| MIDI 匯出 | `midi_writer.hpp` | 在記憶體中產生 Standard MIDI 檔 |
| MusicXML 匯出 | `musicxml_writer.hpp` | 量化主旋律譜，含和弦符號、歌詞、調號、拍號 |
| 音訊 I/O | `audio_io.hpp` | WAV 讀寫與單聲道降混 |
| 預覽渲染 | `audio_renderer.hpp` | 簡易振盪器合成 WAV，用於快速驗收 |
| 整合管線 | `core_pipeline.hpp` | 將上述模組串接為端對端流程 |
| C ABI 介面 | `c_api.h` | 對外的 C 介面，便於跨語言整合 |

---

## 目前限制

目前實作鎖定於**乾淨的單聲道人聲或單旋律輸入**。下列功能尚未涵蓋，建議透過 `model_interfaces.hpp` 所定義的擴充點以模型導向的方式後續補上：

- 來源分離（Source Separation）
- 未知歌詞自動語音辨識（ASR）
- 神經網路式 Audio-to-MIDI
- 複音音高偵測
- 完整管弦樂配器

模型導向擴充的 schema 詳見 `docs/model_integration_schemas.md`。

---

## 授權條款

本專案以 **Apache License 2.0** 授權釋出。

```
Copyright (c) Music Elf contributors

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
```

完整授權條文請參閱 [LICENSE](./LICENSE)（如尚未建立，請至 <https://www.apache.org/licenses/LICENSE-2.0.txt> 取得官方版本）。

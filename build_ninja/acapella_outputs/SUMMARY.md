# Acapella Real-Audio Test Summary

Total fixtures: 7
Total sub-tests: 177
Failures: 0

| Song | SR | Dur(s) | Key | BPM | Notes | Pitch cents | Stable % | MIDI B | XML chars |
| --- | ---:| ---:| --- | ---:| ---:| ---:| ---:| ---:| ---:|
| twinkle_fast | 8000 | 23 | Ab minor | 133.9 | 82 | 21 | 62.2 | 775 | 22949 |
| twinkle_slow | 8000 | 64.2 | F minor | 133.9 | 236 | 28.9 | 37 | 2085 | 62321 |
| mary_lamb | 8000 | 82.6 | Bb minor | 133.9 | 328 | 24.7 | 50.4 | 2910 | 85376 |
| london_bridge | 8000 | 33.1 | Eb minor | 136.3 | 161 | 25.7 | 49 | 1452 | 38594 |
| lightly_row | 8000 | 30.7 | E minor | 133.9 | 132 | 27 | 44.1 | 1211 | 32691 |
| happy_birthday | 8000 | 75.3 | D minor | 129.3 | 273 | 24.4 | 51.3 | 2440 | 71112 |
| happy_birthday_thomas_48k | 48000 | 13.8 | E minor | 140.6 | 57 | 24.4 | 50.8 | 560 | 15672 |

## Sub-test log

```
[OK] twinkle_fast :: audio_io.read_wav_file -- rate=8000 samples=184404
[OK] twinkle_fast :: audio_io.downmix_to_mono -- mono_samples=184404
[OK] twinkle_fast :: pitch_detector.process -- frames=2874 voiced_ratio=0.899443
[OK] twinkle_fast :: pitch_detector.stability_reported -- mean_abs_cents=21.050736 stable_frames=1609 stable_ratio=0.622437
[OK] twinkle_fast :: pitch_detector.voiced_ratio -- expected>0.30 got=0.899443
[OK] twinkle_fast :: pitch_detector.median_midi -- expected_pc=4 got_pc=9 (midi=69.000000)
[OK] twinkle_fast :: note_segmenter.monotonic_starts
[OK] twinkle_fast :: note_segmenter.positive_duration
[OK] twinkle_fast :: note_segmenter.note_count -- expected=[10,200] got=82
[OK] twinkle_fast :: note_segmenter.median_pitch -- expected_pc=4 got_pc=9 (midi=69)
[OK] twinkle_fast :: rhythm.bpm_finite -- bpm=133.928571
[OK] twinkle_fast :: rhythm.bpm_range -- expected=[60.000000,180.000000] got=133.928571
[OK] twinkle_fast :: rhythm.quantized_monotonic -- quantized_notes=82
[OK] twinkle_fast :: dynamics.size_matches_notes -- notes=82 dynamics=82
[OK] twinkle_fast :: dynamics.velocity_range
[OK] twinkle_fast :: dynamics.rms_finite
[OK] twinkle_fast :: dynamics.active_velocity_ratio -- got=0.987805
[OK] twinkle_fast :: harmony.progressions_nonempty -- count=5
[OK] twinkle_fast :: harmony.detected_key -- expected_pc=4 got_pc=8 (Ab minor)
[OK] twinkle_fast :: lyric_aligner.size -- expected=10 got=10
[OK] twinkle_fast :: lyric_aligner.indices_increasing
[OK] twinkle_fast :: midi_writer.header -- bytes=775
[OK] twinkle_fast :: musicxml_writer.structure -- chars=22949
[OK] twinkle_fast :: core_pipeline.run_core_pipeline -- notes=82 midi_bytes=1048
[OK] twinkle_fast :: audio_renderer.render_notes_to_audio -- samples=184992
[OK] twinkle_fast :: outputs_persisted -- dir=C:/Users/user/Documents/app/music_elf/build_ninja/acapella_outputs\twinkle_fast
[OK] twinkle_slow :: audio_io.read_wav_file -- rate=8000 samples=513623
[OK] twinkle_slow :: audio_io.downmix_to_mono -- mono_samples=513623
[OK] twinkle_slow :: pitch_detector.process -- frames=8018 voiced_ratio=0.834747
[OK] twinkle_slow :: pitch_detector.stability_reported -- mean_abs_cents=28.951962 stable_frames=2481 stable_ratio=0.370686
[OK] twinkle_slow :: pitch_detector.voiced_ratio -- expected>0.30 got=0.834747
[OK] twinkle_slow :: pitch_detector.median_midi -- expected_pc=4 got_pc=5 (midi=65.000000)
[OK] twinkle_slow :: note_segmenter.monotonic_starts
[OK] twinkle_slow :: note_segmenter.positive_duration
[OK] twinkle_slow :: note_segmenter.note_count -- expected=[10,400] got=236
[OK] twinkle_slow :: note_segmenter.median_pitch -- expected_pc=4 got_pc=5 (midi=65)
[OK] twinkle_slow :: rhythm.bpm_finite -- bpm=133.928571
[OK] twinkle_slow :: rhythm.bpm_range -- expected=[40.000000,200.000000] got=133.928571
[OK] twinkle_slow :: rhythm.quantized_monotonic -- quantized_notes=236
[OK] twinkle_slow :: dynamics.size_matches_notes -- notes=236 dynamics=236
[OK] twinkle_slow :: dynamics.velocity_range
[OK] twinkle_slow :: dynamics.rms_finite
[OK] twinkle_slow :: dynamics.active_velocity_ratio -- got=0.995763
[OK] twinkle_slow :: harmony.progressions_nonempty -- count=5
[OK] twinkle_slow :: harmony.detected_key -- expected_pc=4 got_pc=5 (F minor)
[OK] twinkle_slow :: lyric_aligner.size -- expected=10 got=10
[OK] twinkle_slow :: lyric_aligner.indices_increasing
[OK] twinkle_slow :: midi_writer.header -- bytes=2085
[OK] twinkle_slow :: musicxml_writer.structure -- chars=62321
[OK] twinkle_slow :: core_pipeline.run_core_pipeline -- notes=236 midi_bytes=3012
[OK] twinkle_slow :: audio_renderer.render_notes_to_audio -- samples=509281
[OK] twinkle_slow :: outputs_persisted -- dir=C:/Users/user/Documents/app/music_elf/build_ninja/acapella_outputs\twinkle_slow
[OK] mary_lamb :: audio_io.read_wav_file -- rate=8000 samples=660933
[OK] mary_lamb :: audio_io.downmix_to_mono -- mono_samples=660933
[OK] mary_lamb :: pitch_detector.process -- frames=10320 voiced_ratio=0.752132
[OK] mary_lamb :: pitch_detector.stability_reported -- mean_abs_cents=24.770922 stable_frames=3919 stable_ratio=0.504896
[OK] mary_lamb :: pitch_detector.voiced_ratio -- expected>0.30 got=0.752132
[OK] mary_lamb :: pitch_detector.median_midi -- expected_pc=8 got_pc=9 (midi=69.000000)
[OK] mary_lamb :: note_segmenter.monotonic_starts
[OK] mary_lamb :: note_segmenter.positive_duration
[OK] mary_lamb :: note_segmenter.note_count -- expected=[8,400] got=328
[OK] mary_lamb :: note_segmenter.median_pitch -- expected_pc=8 got_pc=9 (midi=69)
[OK] mary_lamb :: rhythm.bpm_finite -- bpm=133.928571
[OK] mary_lamb :: rhythm.bpm_range -- expected=[60.000000,180.000000] got=133.928571
[OK] mary_lamb :: rhythm.quantized_monotonic -- quantized_notes=328
[OK] mary_lamb :: dynamics.size_matches_notes -- notes=328 dynamics=328
[OK] mary_lamb :: dynamics.velocity_range
[OK] mary_lamb :: dynamics.rms_finite
[OK] mary_lamb :: dynamics.active_velocity_ratio -- got=0.996951
[OK] mary_lamb :: harmony.progressions_nonempty -- count=5
[OK] mary_lamb :: harmony.detected_key -- expected_pc=10 got_pc=10 (Bb minor)
[OK] mary_lamb :: lyric_aligner.size -- expected=9 got=9
[OK] mary_lamb :: lyric_aligner.indices_increasing
[OK] mary_lamb :: midi_writer.header -- bytes=2910
[OK] mary_lamb :: musicxml_writer.structure -- chars=85376
[OK] mary_lamb :: core_pipeline.run_core_pipeline -- notes=328 midi_bytes=4043
[OK] mary_lamb :: audio_renderer.render_notes_to_audio -- samples=657376
[OK] mary_lamb :: outputs_persisted -- dir=C:/Users/user/Documents/app/music_elf/build_ninja/acapella_outputs\mary_lamb
[OK] london_bridge :: audio_io.read_wav_file -- rate=8000 samples=265138
[OK] london_bridge :: audio_io.downmix_to_mono -- mono_samples=265138
[OK] london_bridge :: pitch_detector.process -- frames=4135 voiced_ratio=0.861427
[OK] london_bridge :: pitch_detector.stability_reported -- mean_abs_cents=25.727856 stable_frames=1746 stable_ratio=0.490174
[OK] london_bridge :: pitch_detector.voiced_ratio -- expected>0.30 got=0.861427
[OK] london_bridge :: pitch_detector.median_midi -- expected_pc=11 got_pc=4 (midi=64.000000)
[OK] london_bridge :: note_segmenter.monotonic_starts
[OK] london_bridge :: note_segmenter.positive_duration
[OK] london_bridge :: note_segmenter.note_count -- expected=[8,300] got=161
[OK] london_bridge :: note_segmenter.median_pitch -- expected_pc=11 got_pc=4 (midi=64)
[OK] london_bridge :: rhythm.bpm_finite -- bpm=136.363636
[OK] london_bridge :: rhythm.bpm_range -- expected=[60.000000,180.000000] got=136.363636
[OK] london_bridge :: rhythm.quantized_monotonic -- quantized_notes=161
[OK] london_bridge :: dynamics.size_matches_notes -- notes=161 dynamics=161
[OK] london_bridge :: dynamics.velocity_range
[OK] london_bridge :: dynamics.rms_finite
[OK] london_bridge :: dynamics.active_velocity_ratio -- got=0.993789
[OK] london_bridge :: harmony.progressions_nonempty -- count=5
[OK] london_bridge :: harmony.detected_key -- expected_pc=11 got_pc=3 (Eb minor)
[OK] london_bridge :: lyric_aligner.size -- expected=9 got=9
[OK] london_bridge :: lyric_aligner.indices_increasing
[OK] london_bridge :: midi_writer.header -- bytes=1452
[OK] london_bridge :: musicxml_writer.structure -- chars=38594
[OK] london_bridge :: core_pipeline.run_core_pipeline -- notes=161 midi_bytes=1890
[OK] london_bridge :: audio_renderer.render_notes_to_audio -- samples=258017
[OK] london_bridge :: outputs_persisted -- dir=C:/Users/user/Documents/app/music_elf/build_ninja/acapella_outputs\london_bridge
[OK] lightly_row :: audio_io.read_wav_file -- rate=8000 samples=246189
[OK] lightly_row :: audio_io.downmix_to_mono -- mono_samples=246189
[OK] lightly_row :: pitch_detector.process -- frames=3839 voiced_ratio=0.765824
[OK] lightly_row :: pitch_detector.stability_reported -- mean_abs_cents=27.060675 stable_frames=1299 stable_ratio=0.441837
[OK] lightly_row :: pitch_detector.voiced_ratio -- expected>0.30 got=0.765824
[OK] lightly_row :: pitch_detector.median_midi -- expected_pc=8 got_pc=4 (midi=64.000000)
[OK] lightly_row :: note_segmenter.monotonic_starts
[OK] lightly_row :: note_segmenter.positive_duration
[OK] lightly_row :: note_segmenter.note_count -- expected=[10,300] got=132
[OK] lightly_row :: note_segmenter.median_pitch -- expected_pc=8 got_pc=4 (midi=64)
[OK] lightly_row :: rhythm.bpm_finite -- bpm=133.928571
[OK] lightly_row :: rhythm.bpm_range -- expected=[60.000000,180.000000] got=133.928571
[OK] lightly_row :: rhythm.quantized_monotonic -- quantized_notes=132
[OK] lightly_row :: dynamics.size_matches_notes -- notes=132 dynamics=132
[OK] lightly_row :: dynamics.velocity_range
[OK] lightly_row :: dynamics.rms_finite
[OK] lightly_row :: dynamics.active_velocity_ratio -- got=0.992424
[OK] lightly_row :: harmony.progressions_nonempty -- count=5
[OK] lightly_row :: harmony.detected_key -- expected_pc=1 got_pc=4 (E minor)
[OK] lightly_row :: lyric_aligner.size -- expected=10 got=10
[OK] lightly_row :: lyric_aligner.indices_increasing
[OK] lightly_row :: midi_writer.header -- bytes=1211
[OK] lightly_row :: musicxml_writer.structure -- chars=32691
[OK] lightly_row :: core_pipeline.run_core_pipeline -- notes=132 midi_bytes=1559
[OK] lightly_row :: audio_renderer.render_notes_to_audio -- samples=243360
[OK] lightly_row :: outputs_persisted -- dir=C:/Users/user/Documents/app/music_elf/build_ninja/acapella_outputs\lightly_row
[OK] happy_birthday :: audio_io.read_wav_file -- rate=8000 samples=602790
[OK] happy_birthday :: audio_io.downmix_to_mono -- mono_samples=602790
[OK] happy_birthday :: pitch_detector.process -- frames=9411 voiced_ratio=0.810435
[OK] happy_birthday :: pitch_detector.stability_reported -- mean_abs_cents=24.469291 stable_frames=3914 stable_ratio=0.513177
[OK] happy_birthday :: pitch_detector.voiced_ratio -- expected>0.30 got=0.810435
[OK] happy_birthday :: pitch_detector.median_midi -- expected_pc=9 got_pc=9 (midi=69.000000)
[OK] happy_birthday :: note_segmenter.monotonic_starts
[OK] happy_birthday :: note_segmenter.positive_duration
[OK] happy_birthday :: note_segmenter.note_count -- expected=[8,400] got=273
[OK] happy_birthday :: note_segmenter.median_pitch -- expected_pc=9 got_pc=8 (midi=68)
[OK] happy_birthday :: rhythm.bpm_finite -- bpm=129.310345
[OK] happy_birthday :: rhythm.bpm_range -- expected=[60.000000,180.000000] got=129.310345
[OK] happy_birthday :: rhythm.quantized_monotonic -- quantized_notes=273
[OK] happy_birthday :: dynamics.size_matches_notes -- notes=273 dynamics=273
[OK] happy_birthday :: dynamics.velocity_range
[OK] happy_birthday :: dynamics.rms_finite
[OK] happy_birthday :: dynamics.active_velocity_ratio -- got=0.996337
[OK] happy_birthday :: harmony.progressions_nonempty -- count=5
[OK] happy_birthday :: harmony.detected_key -- expected_pc=9 got_pc=2 (D minor)
[OK] happy_birthday :: lyric_aligner.size -- expected=8 got=8
[OK] happy_birthday :: lyric_aligner.indices_increasing
[OK] happy_birthday :: midi_writer.header -- bytes=2440
[OK] happy_birthday :: musicxml_writer.structure -- chars=71112
[OK] happy_birthday :: core_pipeline.run_core_pipeline -- notes=273 midi_bytes=3479
[OK] happy_birthday :: audio_renderer.render_notes_to_audio -- samples=598432
[OK] happy_birthday :: outputs_persisted -- dir=C:/Users/user/Documents/app/music_elf/build_ninja/acapella_outputs\happy_birthday
[OK] happy_birthday_thomas_48k :: audio_io.read_wav_file -- rate=48000 samples=1332182
[OK] happy_birthday_thomas_48k :: audio_io.downmix_to_mono -- mono_samples=666091
[OK] happy_birthday_thomas_48k :: pitch_detector.process -- frames=5196 voiced_ratio=0.810431
[OK] happy_birthday_thomas_48k :: pitch_detector.stability_reported -- mean_abs_cents=24.486884 stable_frames=2143 stable_ratio=0.508905
[OK] happy_birthday_thomas_48k :: note_segmenter.monotonic_starts
[OK] happy_birthday_thomas_48k :: note_segmenter.positive_duration
[OK] happy_birthday_thomas_48k :: note_segmenter.note_count -- expected=[0,4096] got=57
[OK] happy_birthday_thomas_48k :: rhythm.bpm_finite -- bpm=140.625000
[OK] happy_birthday_thomas_48k :: rhythm.quantized_monotonic -- quantized_notes=57
[OK] happy_birthday_thomas_48k :: dynamics.size_matches_notes -- notes=57 dynamics=57
[OK] happy_birthday_thomas_48k :: dynamics.velocity_range
[OK] happy_birthday_thomas_48k :: dynamics.rms_finite
[OK] happy_birthday_thomas_48k :: dynamics.active_velocity_ratio -- got=0.982456
[OK] happy_birthday_thomas_48k :: harmony.progressions_nonempty -- count=5
[OK] happy_birthday_thomas_48k :: lyric_aligner.size -- expected=8 got=8
[OK] happy_birthday_thomas_48k :: lyric_aligner.indices_increasing
[OK] happy_birthday_thomas_48k :: midi_writer.header -- bytes=560
[OK] happy_birthday_thomas_48k :: musicxml_writer.structure -- chars=15672
[OK] happy_birthday_thomas_48k :: core_pipeline.run_core_pipeline -- notes=57 midi_bytes=753
[OK] happy_birthday_thomas_48k :: audio_renderer.render_notes_to_audio -- samples=662976
[OK] happy_birthday_thomas_48k :: outputs_persisted -- dir=C:/Users/user/Documents/app/music_elf/build_ninja/acapella_outputs\happy_birthday_thomas_48k
```

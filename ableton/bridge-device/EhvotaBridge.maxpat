{
  "patcher": {
    "fileversion": 1,
    "appversion": { "major": 9, "minor": 0, "revision": 0, "architecture": "x64", "modernui": 1 },
    "classnamespace": "box",
    "rect": [80.0, 80.0, 720.0, 360.0],
    "openinpresentation": 1,
    "default_fontsize": 12.0,
    "default_fontface": 0,
    "default_fontname": "Arial",
    "boxes": [
      { "box": { "id": "in-1", "maxclass": "inlet", "patching_rect": [30.0, 30.0, 30.0, 30.0], "comment": "Read-only bridge commands" } },
      { "box": { "id": "js-1", "maxclass": "newobj", "text": "js ehvota_bridge.js", "patching_rect": [150.0, 118.0, 160.0, 22.0] } },
      { "box": { "id": "out-1", "maxclass": "outlet", "patching_rect": [420.0, 100.0, 30.0, 30.0], "comment": "JSON state snapshots" } },
      { "box": { "id": "out-2", "maxclass": "outlet", "patching_rect": [500.0, 145.0, 30.0, 30.0], "comment": "Heartbeat and diagnostics" } },
      { "box": { "id": "loadbang-1", "maxclass": "newobj", "text": "loadbang", "patching_rect": [30.0, 92.0, 60.0, 22.0] } },
      { "box": { "id": "trigger-1", "maxclass": "newobj", "text": "t b b", "patching_rect": [30.0, 128.0, 42.0, 22.0] } },
      { "box": { "id": "snapshot-msg", "maxclass": "message", "text": "snapshot", "patching_rect": [82.0, 164.0, 62.0, 22.0] } },
      { "box": { "id": "toggle-1", "maxclass": "toggle", "patching_rect": [30.0, 164.0, 24.0, 24.0] } },
      { "box": { "id": "qmetro-1", "maxclass": "newobj", "text": "qmetro 1000", "patching_rect": [30.0, 204.0, 82.0, 22.0] } },
      { "box": { "id": "heartbeat-msg", "maxclass": "message", "text": "heartbeat", "patching_rect": [30.0, 240.0, 66.0, 22.0] } },
      { "box": { "id": "title", "maxclass": "comment", "text": "Ehvota Bridge · read-only Live Object Model bridge", "patching_rect": [190.0, 35.0, 310.0, 22.0], "presentation": 1, "presentation_rect": [16.0, 14.0, 320.0, 22.0] } },
      { "box": { "id": "status", "maxclass": "comment", "text": "Publishes snapshots and heartbeat. No audio processing. No write commands.", "patching_rect": [190.0, 62.0, 400.0, 22.0], "presentation": 1, "presentation_rect": [16.0, 42.0, 430.0, 22.0] } }
    ],
    "lines": [
      { "patchline": { "source": ["in-1", 0], "destination": ["js-1", 0] } },
      { "patchline": { "source": ["js-1", 0], "destination": ["out-1", 0] } },
      { "patchline": { "source": ["js-1", 1], "destination": ["out-2", 0] } },
      { "patchline": { "source": ["loadbang-1", 0], "destination": ["trigger-1", 0] } },
      { "patchline": { "source": ["trigger-1", 1], "destination": ["snapshot-msg", 0] } },
      { "patchline": { "source": ["snapshot-msg", 0], "destination": ["js-1", 0] } },
      { "patchline": { "source": ["trigger-1", 0], "destination": ["toggle-1", 0] } },
      { "patchline": { "source": ["toggle-1", 0], "destination": ["qmetro-1", 0] } },
      { "patchline": { "source": ["qmetro-1", 0], "destination": ["heartbeat-msg", 0] } },
      { "patchline": { "source": ["heartbeat-msg", 0], "destination": ["js-1", 0] } }
    ]
  }
}

# Kirikiri Z File Patch

The universal patch extension for some new Kirikiri game.

## How To Use

You need to create `KrkrPatch.json` with the following format.

```json
{
  "gameExecutableFile": "your_game.exe",
  "gameCommandLine": "",
  "logLevel": 0,
  "patchProtocols": [
    "arc://",
    "archive://",
    "psb://"
  ],
  "patchArchives": [
    "your_patch.xp3"
  ]
}
```

Put `KrkrPatchLoader.exe` and `KrkrPatch.dll` and `KrkrPatch.json` to your game folder.

Use `GARbro` create your `patch.xp3` with `No Crypt` and `keep directory structure`.

Finally, Run the `KrkrPatchLoader`.

## Notes

You can rename `KrkrPatchLoader` or change the icon and version information with some tool like `Resource Hacker`.

Don't rename `KrkrPatch.dll` and `KrkrPatch.json`.

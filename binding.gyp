{
  "targets": [
    {
      "include_dirs" : [
        "<!(node -e \"require('nan')\")"
      ],
      "target_name": "addon",
      "sources": [ 
          "src/addon.cpp",
      ],
      "cflags_cc!": [ '-fno-rtti', '-fno-exceptions' ]
    }
  ]
}

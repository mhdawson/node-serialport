{
  'targets': [{
    'target_name': 'serialport',
      'cflags!': [ '-fno-exceptions' ],
      'cflags_cc!': [ '-fno-exceptions' ],
    'sources': [
      'src/serialport.cpp'
    ],
    'include_dirs': [
      '<!@(node -p "require(\'node-addon-api\').include")'
    ],
    'conditions': [
      ['OS=="win"',
        {
          'sources': [
            'src/serialport_win.cpp'
          ],
          'msvs_settings': {
            'VCCLCompilerTool': {
              'ExceptionHandling': '2',
              'DisableSpecificWarnings': [ '4530', '4506' ],
            }
          }
        }
      ],
      ['OS=="mac"',
        {
          'sources': [
            'src/serialport_unix.cpp',
            'src/poller.cpp'
          ],
          'xcode_settings': {
            'OTHER_LDFLAGS': [
              '-framework CoreFoundation -framework IOKit'
            ]
          }
        }
      ],
      ['OS!="win"',
        {
          'sources': [
            'src/serialport_unix.cpp',
            'src/poller.cpp'
          ]
        }
      ]
    ]
  }],
}

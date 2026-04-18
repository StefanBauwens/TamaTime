module.exports = [
  { 
    "type": "heading", 
    "defaultValue": "TamaTime" 
  }, 
  { 
    "type": "text", 
    "defaultValue": "Created by Stefan Bauwens as a companion watchface to the <a href='https://apps.repebble.com/tamagotchi-emulator_216a0f62c6e44aac8f725e68'>Tamagotchi Emulator 4 Pebble watchapp.</a>" 
  },
  { 
    "type": "text", 
    "defaultValue": "For more info check the readme at <a href='https://github.com/StefanBauwens/Tamagotchi-Emulator-Pebble'>https://github.com/StefanBauwens/Tamagotchi-Emulator-Pebble</a>" 
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Settings"
      },
      {
        "type": "toggle",
        "messageKey": "UseSeconds",
        "label": "Use seconds",
        "defaultValue": true
      },
      {
        "type": "text",
        "defaultValue": "(Optional) Tamagotchi Server used by Tamagotchi Emulator 4 Pebble. Leave blank if unused."
      },
      {
        "type": "input",
        "messageKey": "APIServerUrl",
        "label": "Server",
        "defaultValue": "",
        "attributes": {
          "placeholder": "e.g. http://192.168.0.100:5000"
        }
      },
      {
        "type": "toggle",
        "messageKey": "SwapScreens",
        "label": "Swap screens",
        "defaultValue": false
      },
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save"
  }
]
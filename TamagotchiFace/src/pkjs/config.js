module.exports = [
  { 
    "type": "heading", 
    "defaultValue": "Tamagotchi Helper Face" 
  }, 
  { 
    "type": "text", 
    "defaultValue": "Created by Stefan Bauwens as a companion watchface to the Tamagotchi Emulator 4 Pebble watchapp." 
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
        "type": "text",
        "defaultValue": "Tamagotchi Server used by Tamagotchi Emulator 4 Pebble"
      },
      {
        "type": "input",
        "messageKey": "APIServerUrl",
        "label": "Server",
        "defaultValue": "",
        "attributes": {
          "placeholder": "e.g. http://192.168.0.100:5000"
        }
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save"
  }
]
# SDLogger Library 
This library provides a useful interface for saving and retrieving time stamped data from an SD card using the Arduino platform. This code was originally developed and tested for an ESP32 micro-controller.


## Modules
There are two modules with separate but related functions present in this library. The first is the SDLogger which provides the framework for storing data to an SD card in CSV files with timestamps as the key and the assumption that the logged data is originally MQTT JSON formatted.

The second module is the SDReader, which is written to retrieve the same data stored by the SDLogger from CSV files. Data may be selected by date range(timestamps) and filtered by MQTT topic string matching.

Example data from a sample CSV file is listed below:

```csv
TIME;MQTT TOPIC;MQTT MESSAGE;
5-24-2023T17:10:57+0;meter_teros10/0_shallow/08:3A:F2:31:9B:D0;{"MAC": "08:3A:F2:31:9B:D0", "DEPTH": "shallow", "VWC_RAW":0.003000, "VWC":-2.142326};
5-24-2023T17:10:57+0;meter_teros10/1_middle/08:3A:F2:31:9B:D0;{"MAC": "08:3A:F2:31:9B:D0", "DEPTH": "middle", "VWC_RAW":0.003000, "VWC":-2.142326};
5-24-2023T17:10:57+0;meter_teros10/2_deep/08:3A:F2:31:9B:D0;{"MAC": "08:3A:F2:31:9B:D0", "DEPTH": "deep", "VWC_RAW":0.003000, "VWC":-2.142326};
5-24-2023T17:10:57+0;kkm_k6p/bc:57:29:00:f6:d3;{"MAC": "bc:57:29:00:f6:d3", "HUMIDITY": 40.167999, "TEMP": 21.136999, "GATOR_MAC": "08:3A:F2:31:9B:D0"};
```

## Example Usage
You can find an example of the SDReader module usage in the examples folder.


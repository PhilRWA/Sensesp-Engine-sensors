// Signal K application template file.
//
// This application demonstrates core SensESP concepts in a very
// concise manner. You can build and upload the application as is
// and observe the value changes on the serial port monitor.
//
// You can use this source file as a basis for your own projects.
// Remove the parts that are not relevant to you, and add your own code
// for external hardware libraries.

#include <Adafruit_BMP280.h>
#include <Wire.h>
#include "sensesp_onewire/onewire_temperature.h"
#include <Arduino.h>
#include "sensesp/sensors/analog_input.h"
#include "sensesp/sensors/digital_input.h"
#include "sensesp/sensors/sensor.h"
#include "sensesp/signalk/signalk_output.h"
#include "sensesp/system/lambda_consumer.h"
#include "sensesp_app_builder.h"
#include "sensesp/transforms/linear.h"
#include "sensesp/transforms/analogvoltage.h"
#include "sensesp/transforms/curveinterpolator.h"
#include "sensesp/transforms/voltagedivider.h"
#include "sensesp/transforms/frequency.h"
#include "sensesp/transforms/moving_average.h"

using namespace sensesp;

class TemperatureInterpreter : public CurveInterpolator {
 public:
  TemperatureInterpreter(String config_path = "")
      : CurveInterpolator(NULL, config_path) {
    // Populate a lookup table to translate the ohm values returned by
    // our temperature sender to degrees Kelvin
    clear_samples();
    // addSample(CurveInterpolator::Sample(knownOhmValue, knownKelvin));
    add_sample(CurveInterpolator::Sample(1743.15, 273.15));
    add_sample(CurveInterpolator::Sample(1075.63, 283.15));
    add_sample(CurveInterpolator::Sample(676.95, 293.15));
    add_sample(CurveInterpolator::Sample(439.29, 303.15));
    add_sample(CurveInterpolator::Sample(291.46, 313.15));
    add_sample(CurveInterpolator::Sample(197.29, 323.15));
    add_sample(CurveInterpolator::Sample(134.03, 333.15));
    add_sample(CurveInterpolator::Sample(97.05, 343.15));
    add_sample(CurveInterpolator::Sample(70.12, 353.15));
    add_sample(CurveInterpolator::Sample(51.21, 363.15));
    add_sample(CurveInterpolator::Sample(38.47, 373.15));
    add_sample(CurveInterpolator::Sample(29.12, 383.15)); 
  }
};

class FuelInterpreter : public CurveInterpolator {
 public:
  FuelInterpreter(String config_path = "")
      : CurveInterpolator(NULL, config_path) {
    // Populate a lookup table to translate RPM to LPH
    clear_samples();
    // addSample(CurveInterpolator::Sample(RPM, LPH));
    add_sample(CurveInterpolator::Sample(1200, 2.3));
    add_sample(CurveInterpolator::Sample(1500, 2.98));
    add_sample(CurveInterpolator::Sample(1800, 3.4));
    add_sample(CurveInterpolator::Sample(2100, 3.95));
    add_sample(CurveInterpolator::Sample(2400, 4.3));
    add_sample(CurveInterpolator::Sample(2700, 4.73));
    add_sample(CurveInterpolator::Sample(3000, 5.17));
    add_sample(CurveInterpolator::Sample(3300, 5.5));
    add_sample(CurveInterpolator::Sample(3600, 5.7));  
  }
};

reactesp::ReactESP app;

Adafruit_BMP280 bmp280;

  float read_temp_callback() { return (bmp280.readTemperature() + 273.15);}
  float read_pressure_callback() { return (bmp280.readPressure());}

// The setup function performs one-time application initialization.
void setup() {
#ifndef SERIAL_DEBUG_DISABLED
  SetupSerialDebug(115200);
#endif

  // Construct the global SensESPApp() object
  SensESPAppBuilder builder;
  sensesp_app = (&builder)
                    // Set a custom hostname for the app.
                    ->set_hostname("Engine digitiser")
                    // Optionally, hard-code the WiFi and Signal K server
                    // settings. This is normally not needed.
                    //->set_wifi("AKC Sound 03", "Outwitting")
                    //->set_sk_server("10.0.0.11", 3000)
                    ->get_app();

/// 1-Wire Temp Sensors - Exhaust Temp Sensors ///

  DallasTemperatureSensors* dts = new DallasTemperatureSensors(17);

  auto* exhaust_temp =
      new OneWireTemperature(dts, 1000, "/Exhaust Temperature/oneWire");
 
  exhaust_temp->connect_to(new Linear(1.0, 0.0, "/Exhaust Temperature/linear"))
      ->connect_to(new SKOutputFloat("propulsion.main.exhaustTemperature","/Exhaust Temperature/sk_path"));

  auto* calorifier_temp =
      new OneWireTemperature(dts, 1000, "/Calorifier Temperature/oneWire");
 
  calorifier_temp->connect_to(new Linear(1.0, 0.0, "/Calorifier Temperature/linear"))
      ->connect_to(new SKOutputFloat("environemnt.calorifierTemperature","/Calorifier Temperature/sk_path"));

  auto* engine_room_temp =
      new OneWireTemperature(dts, 1000, "/Engine Room Temperature/oneWire");
 
  engine_room_temp->connect_to(new Linear(1.0, 0.0, "/Engine Room Temperature/linear"))
      ->connect_to(new SKOutputFloat("enviroment.engineRoomTemperature","/Engine Room Temperature/sk_path"));
      
      //// Engine Temp Config ////

const float Vin = 3.3;
const float R1 = 2200.0; //Change this for final//
auto* analog_input = new AnalogInput(36, 2000);

analog_input->connect_to(new AnalogVoltage(Vin, Vin))
      ->connect_to(new VoltageDividerR2(R1, Vin, "/Engine Temp/sender"))
      ->connect_to(new TemperatureInterpreter("/Engine Temp/curve"))
      ->connect_to(new Linear(1.0, 0.9, "/Engine Temp/calibrate"))
      ->connect_to(new MovingAverage( 4, 1.0,"/Engine Temp/movingAvg"))
      ->connect_to(new SKOutputFloat("propulsion.engine.temperature", "/Engine Temp/sk_path"));

//RPM Application/////      

const char* config_path_calibrate = "/Engine RPM/calibrate";
  const char* config_path_skpath = "/Engine RPM/sk_path";
  const float multiplier = 1.0;

  auto* sensor = new DigitalInputCounter(16, INPUT_PULLUP, RISING, 500);

  sensor->connect_to(new Frequency(multiplier, config_path_calibrate))  
  // connect the output of sensor to the input of Frequency()
         ->connect_to(new MovingAverage(2, 1.0,"/Engine RPM/movingAVG"))
         ->connect_to(new SKOutputFloat("propulsion.main.revolutions", config_path_skpath));  
          // connect the output of Frequency() to a Signal K Output as a number

  sensor->connect_to(new Frequency(6))
  // times by 6 to go from Hz to RPM
          ->connect_to(new MovingAverage(4, 1.0,"/Engine Fuel/movingAVG"))
          ->connect_to(new FuelInterpreter("/Engine Fuel/curve"))
          ->connect_to(new SKOutputFloat("propulsion.engine.fuelconsumption", "/Engine Fuel/sk_path"));   

   //// Bilge Monitor /////

auto* bilge = new DigitalInputState(25, INPUT_PULLUP, 5000);

auto int_to_string_function = [](int input) ->String {
     if (input == 1) {
       return "Water present!";
     } 
     else { // input == 0
       return "bilge clear";
     }
};

auto int_to_string_transform = new LambdaTransform<int, String>(int_to_string_function);

bilge->connect_to(int_to_string_transform)
      ->connect_to(new SKOutputString("propulsion.engine.bilge"));

bilge->connect_to(new SKOutputString("propulsion.engine.bilge.raw"));

  // Start networking, SK server connections and other SensESP internals
  sensesp_app->start();
}

void loop() { app.tick(); }

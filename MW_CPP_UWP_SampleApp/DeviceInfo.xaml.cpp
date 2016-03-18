//
// DeviceInfo.xaml.cpp
// Implementation of the DeviceInfo class
//

#include "DeviceInfo.xaml.h"
#include "metawear\peripheral\led.h"
#include "metawear\sensor\accelerometer.h"
#include "metawear\sensor\accelerometer_mma8452q.h"
#include "metawear\sensor\accelerometer_bmi160.h"
#include "metawear\sensor\gpio.h"
#include "metawear\sensor\ambientlight_ltr329.h"
#include "metawear\sensor\barometer_bmp280.h"
#include "metawear\sensor\gyro_bmi160.h"
#include "metawear\sensor\switch.h"
#include "metawear\peripheral\haptic.h"
#include "metawear\core\datasignal.h"
#include "metawear\core\types.h"
#include "metawear\core\timer.h"
#include "metawear\core\event.h"
#include "metawear\core\cpp\event_private.h"
#include "metawear\core\cpp\timer_private.h"

#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>


using namespace MW_CPP_UWP_SampleApp;

using namespace std;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::ApplicationModel::Core;
using namespace Platform;
using namespace Platform::Collections;
using namespace concurrency;
using namespace Windows::Storage::Streams;


const GUID GUID_METAWEAR_SERVICE = { 0x326A9000, 0x85CB, 0x9195, 0xD9, 0xDD, 0x46, 0x4C, 0xFB, 0xBA, 0xE7, 0x5A };
const GUID GUID_DEVICE_INFO_SERVICE = { 0x0000180a, 0x0000, 0x1000, 0x80, 0x00, 0x00, 0x80, 0x5f, 0x9b, 0x34, 0xfb };
const GUID CHARACTERISTIC_MANUFACTURER = { 0x00002a29, 0x0000, 0x1000, 0x80, 0x00, 0x00, 0x80, 0x5f, 0x9b, 0x34, 0xfb };
const GUID CHARACTERISTIC_MODEL_NUMBER = { 0x00002a24, 0x0000, 0x1000, 0x80, 0x00, 0x00, 0x80, 0x5f, 0x9b, 0x34, 0xfb };
const GUID CHARACTERISTIC_SERIAL_NUMBER = { 0x00002a25, 0x0000, 0x1000, 0x80, 0x00, 0x00, 0x80, 0x5f, 0x9b, 0x34, 0xfb };
const GUID CHARACTERISTIC_FIRMWARE_REVISION = { 0x00002a26, 0x0000, 0x1000, 0x80, 0x00, 0x00, 0x80, 0x5f, 0x9b, 0x34, 0xfb };
const GUID CHARACTERISTIC_HARDWARE_REVISION = { 0x00002a27, 0x0000, 0x1000, 0x80, 0x00, 0x00, 0x80, 0x5f, 0x9b, 0x34, 0xfb };
const GUID METAWEAR_COMMAND_CHARACTERISTIC = { 0x326A9001, 0x85CB, 0x9195, 0xD9, 0xDD, 0x46, 0x4C, 0xFB, 0xBA, 0xE7, 0x5A };
const GUID METAWEAR_NOTIFY_CHARACTERISTIC = { 0x326A9006, 0x85CB, 0x9195, 0xD9, 0xDD, 0x46, 0x4C, 0xFB, 0xBA, 0xE7, 0x5A };

enum Signal {
	SWITCH,
	ACCELEROMETER,
	BMP280_PRESSURE,
	BMP280_ALTITUDE,
	AMBIENT_LIGHT,
	GYRO,
	GPIO
};

BluetoothLEDevice^ selectedBtleDevice;

Map<Guid, String^> mwDeviceInfoChars;
Map<Guid, String^> DEVICE_INFO_NAMES;
unordered_map<Signal, MblMwDataSignal*> signals;

MblMwBtleConnection btle_conn;
MblMwMetaWearBoard *board;
MblMwTimer* gpio_read_timer;

GattDeviceService^ mwGattService;
GattCharacteristic^ mwNotifyChar;

bool GPIO_Switch_State;

DeviceInfo^ page;


void data_handler() {
	// handler for accData and gyroData vectors after releasing the push button
	int x = 0;
}

void Uuid2HighLow(Guid uuid, uint64_t &high, uint64_t &low) {
	String^ str = uuid.ToString();
	wstring wstr = str->Data();
	string hx(wstr.begin(), wstr.end());

	for (int i = 0; i < hx.size(); i++)
		if (hx[i] == '-' || hx[i] == '{' || hx[i] == '}') hx[i] = ' ';

	istringstream iss(hx);
	string c1, c2, p1, p2, p3, p4, p5;
	iss >> p1 >> p2 >> p3 >> p4 >> p5;
	c1 = p1 + p2 + p3;
	c2 = p4 + p5;
	sscanf_s(c1.c_str(), "%llx", &high);
	sscanf_s(c2.c_str(), "%llx", &low);
}

Guid HighLow2Uuid(const uint64_t high, const uint64_t low) {
	GUID guid;
	stringstream sstream;
	sstream << std::hex << high;
	sstream << std::hex << low;
	string s = sstream.str();


	unsigned long p0;
	int p1, p2, p3, p4, p5, p6, p7, p8, p9, p10;

	int err = sscanf_s(s.c_str(), "%08lX%04X%04X%02X%02X%02X%02X%02X%02X%02X%02X",
		&p0, &p1, &p2, &p3, &p4, &p5, &p6, &p7, &p8, &p9, &p10);

	guid.Data1 = p0;
	guid.Data2 = p1;
	guid.Data3 = p2;
	guid.Data4[0] = p3;
	guid.Data4[1] = p4;
	guid.Data4[2] = p5;
	guid.Data4[3] = p6;
	guid.Data4[4] = p7;
	guid.Data4[5] = p8;
	guid.Data4[6] = p9;
	guid.Data4[7] = p10;

	return guid;
}


void initialized() {
	if (CoreApplication::MainView->CoreWindow->Dispatcher->HasThreadAccess) {
		page->IsEnabled = true;
	} else {
		CoreApplication::MainView->CoreWindow->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([](void) {
			page->IsEnabled = true;
		}));
	}
}

void write_gatt_char(const MblMwGattChar *characteristic, const uint8_t *value, uint8_t length) {
	// Code to write the value to the characteristic goes here	
	Guid guid = HighLow2Uuid(characteristic->uuid_high, characteristic->uuid_low);
	GattCharacteristic^ mwCommandChar = mwGattService->GetCharacteristics(guid)->First()->Current;

	DataWriter^ writer = ref new DataWriter();
	Array<byte>^ managedArray = ref new Array<byte>((unsigned char*)value, unsigned int(length));
	writer->WriteBytes(managedArray);

	create_task(mwCommandChar->WriteValueAsync(writer->DetachBuffer(), GattWriteOption::WriteWithoutResponse)).then([](GattCommunicationStatus status) {
		if (status != GattCommunicationStatus::Success) {
			// error
		}
	});
}

void read_gatt_char(const MblMwGattChar *characteristic) {
	// Code to read the value from the characteristic goes here
	int x = 0;
}

DeviceInfo::DeviceInfo() {
	InitializeComponent();

	this->NavigationCacheMode = Windows::UI::Xaml::Navigation::NavigationCacheMode::Enabled;

	page = this;

	DEVICE_INFO_NAMES.Insert(CHARACTERISTIC_MANUFACTURER, "Manufacturer");
	DEVICE_INFO_NAMES.Insert(CHARACTERISTIC_MODEL_NUMBER, "Model Number");
	DEVICE_INFO_NAMES.Insert(CHARACTERISTIC_SERIAL_NUMBER, "Serial Number");
	DEVICE_INFO_NAMES.Insert(CHARACTERISTIC_FIRMWARE_REVISION, "Firmware Revision");
	DEVICE_INFO_NAMES.Insert(CHARACTERISTIC_HARDWARE_REVISION, "Hardware Revision");

	btle_conn = { write_gatt_char, read_gatt_char };
	board = mbl_mw_metawearboard_create(&btle_conn);
	mbl_mw_metawearboard_initialize(board, initialized);
}

void DeviceInfo::OnNavigatedTo(NavigationEventArgs^ e) {
	this->IsEnabled = false;
	selectedBtleDevice = (BluetoothLEDevice^)(e->Parameter);
	mwGattService = selectedBtleDevice->GetGattService(GUID_METAWEAR_SERVICE);

	auto chars = selectedBtleDevice->GetGattService(GUID_DEVICE_INFO_SERVICE)->GetAllCharacteristics();
	for each(GattCharacteristic^ ch in chars) {
		create_task(ch->ReadValueAsync()).then([this, ch](GattReadResult^ result) {
			DataReader^ reader = DataReader::FromBuffer(result->Value);
			byte* data = new byte[result->Value->Length];
			reader->ReadBytes(ArrayReference<unsigned char>(data, result->Value->Length));

			String^ value = result->Status == GattCommunicationStatus::Success ? 
				ref new String(wstring(data, data + result->Value->Length).c_str()) : "N/A";
			mwDeviceInfoChars.Insert(ch->Uuid, value);
			outputListView->Items->Append(DEVICE_INFO_NAMES.Lookup(ch->Uuid) + ": " + value);

			MblMwGattChar* mmgc = new MblMwGattChar();
			Uuid2HighLow(ch->Uuid, mmgc->uuid_high, mmgc->uuid_low);
			mbl_mw_connection_char_read(board, mmgc, data, result->Value->Length);
		});
	}

	mwNotifyChar = mwGattService->GetCharacteristics(METAWEAR_NOTIFY_CHARACTERISTIC)->First()->Current;

	create_task(mwNotifyChar->WriteClientCharacteristicConfigurationDescriptorAsync(GattClientCharacteristicConfigurationDescriptorValue::Notify))
		.then([](GattCommunicationStatus status) {
		if (status != GattCommunicationStatus::Success) {
			// error
		}
	});

	mwNotifyChar->ValueChanged += ref new TypedEventHandler<GattCharacteristic^, GattValueChangedEventArgs^>(
		[](GattCharacteristic^ sender, GattValueChangedEventArgs^ obj) {

		DataReader^ reader = DataReader::FromBuffer(obj->CharacteristicValue);
		byte* data = new byte[obj->CharacteristicValue->Length];
		reader->ReadBytes(ArrayReference<unsigned char>(data, obj->CharacteristicValue->Length));
		mbl_mw_connection_notify_char_changed(board, data, obj->CharacteristicValue->Length);
	});
}

void sensors_handler(const MblMwData* data, Signal signal) {
	char buffer[200];
	if (data->value != NULL) {
		switch (signal) {
		case Signal::ACCELEROMETER:
			sprintf_s(buffer, "Acceleration: %.3f, %.3f, %.3f",
				((CartesianFloat*)data->value)->x,
				((CartesianFloat*)data->value)->y,
				((CartesianFloat*)data->value)->z);
			break;
		case Signal::BMP280_ALTITUDE:
			sprintf_s(buffer, "Altitude: %.3fm", *(float*)data->value);
			break;
		case Signal::BMP280_PRESSURE:
			sprintf_s(buffer, "Pressure: %.3fpa", *(float*)data->value);
			break;
		case Signal::AMBIENT_LIGHT:
			sprintf_s(buffer, "Illuminance: %dmlx", *(UINT32*)data->value);
			break;
		case Signal::GYRO:
			sprintf_s(buffer, "Rotation: %.3f, %.3f, %.3f  \u00B0/s",
				((CartesianFloat*)data->value)->x,
				((CartesianFloat*)data->value)->y,
				((CartesianFloat*)data->value)->z);
			break;
		case Signal::SWITCH:
			sprintf_s(buffer, "Switch %s", *(UINT32*)data->value == 0 ? "Released" : "Pressed");			
			break;
		case Signal::GPIO:
			sprintf_s(buffer, "GPIO: %d", *(UINT32*)data->value);
			break;
		default:
			sprintf_s(buffer, "Unexpected signal data");
			break;
		}
			
		String^ newLine = ref new String(wstring(buffer, buffer + strlen(buffer)).c_str());
		if (CoreApplication::MainView->CoreWindow->Dispatcher->HasThreadAccess) {
			page->updateOutputListView(newLine);
		} else {
			CoreApplication::MainView->CoreWindow->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([newLine](void) {
				page->updateOutputListView(newLine);
			}));
		}
	}
}

void acc_handler(const MblMwData* data) { sensors_handler(data, Signal::ACCELEROMETER); }
void baroAlt_handler(const MblMwData* data) { sensors_handler(data, Signal::BMP280_ALTITUDE); }
void baroPre_handler(const MblMwData* data) { sensors_handler(data, Signal::BMP280_PRESSURE); }
void amb_handler(const MblMwData* data) { sensors_handler(data, Signal::AMBIENT_LIGHT); }
void gyro_handler(const MblMwData* data) { sensors_handler(data, Signal::GYRO); }
void switch_handler(const MblMwData* data) { sensors_handler(data, Signal::SWITCH); }
void gpio_handler(const MblMwData* data) { sensors_handler(data, Signal::GPIO); }

void DeviceInfo::startMotor(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e) {
	mbl_mw_haptic_start_motor(board, 100.0, 5000);
}

void DeviceInfo::startBuzzer(Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e) {
	mbl_mw_haptic_start_buzzer(board, 5000);
}

void DeviceInfo::toggleAccelerationSampling(Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e) {
	ToggleSwitch^ toggleSwitch = dynamic_cast<ToggleSwitch^>(sender);

	if (mwDeviceInfoChars.Lookup(CHARACTERISTIC_MODEL_NUMBER)->Equals("0")) {
		if (!signals.count(Signal::ACCELEROMETER)) {
			signals[Signal::ACCELEROMETER] = mbl_mw_acc_mma8452q_get_acceleration_data_signal(board);
		}

		if (toggleSwitch != nullptr) {
			if (toggleSwitch->IsOn) {
				mbl_mw_acc_mma8452q_set_odr(board, MblMwAccMma8452qOdr::MBL_MW_ACC_MMA8452Q_ODR_12_5HZ);
				mbl_mw_acc_mma8452q_set_range(board, MblMwAccMma8452qRange::MBL_MW_ACC_MMA8452Q_FSR_8G);
				mbl_mw_acc_mma8452q_write_acceleration_config(board);

				mbl_mw_datasignal_subscribe(signals[Signal::ACCELEROMETER], acc_handler);
				mbl_mw_acc_mma8452q_enable_acceleration_sampling(board);
				mbl_mw_acc_mma8452q_start(board);
			} else {
				mbl_mw_acc_mma8452q_stop(board);
				mbl_mw_acc_mma8452q_disable_acceleration_sampling(board);
				mbl_mw_datasignal_unsubscribe(signals[Signal::ACCELEROMETER]);
			}
		}
	} else {
		if (!signals.count(Signal::ACCELEROMETER)) {
			signals[Signal::ACCELEROMETER] = mbl_mw_acc_bmi160_get_acceleration_data_signal(board);
		}

		if (toggleSwitch != nullptr) {
			if (toggleSwitch->IsOn) {
				mbl_mw_acc_bmi160_set_odr(board, MblMwAccBmi160Odr::MBL_MW_ACC_BMI160_ODR_12_5HZ);
				mbl_mw_acc_bmi160_set_range(board, MblMwAccBmi160Range::MBL_MW_ACC_BMI160_FSR_8G);
				mbl_mw_acc_bmi160_write_acceleration_config(board);
				mbl_mw_datasignal_subscribe(signals[Signal::ACCELEROMETER], acc_handler);
				mbl_mw_acc_bmi160_enable_acceleration_sampling(board);
				mbl_mw_acc_bmi160_start(board);
			} else {
				mbl_mw_acc_bmi160_stop(board);
				mbl_mw_acc_bmi160_disable_acceleration_sampling(board);
				mbl_mw_datasignal_unsubscribe(signals[Signal::ACCELEROMETER]);
			}
		}
	}
}

void DeviceInfo::toggleBarometerSampling(Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e) {
	ToggleSwitch^ toggleSwitch = dynamic_cast<ToggleSwitch^>(sender);

	if (!signals.count(Signal::BMP280_PRESSURE)) {
		signals[Signal::BMP280_PRESSURE] = mbl_mw_baro_bmp280_get_pressure_data_signal(board);
	}
	if (!signals.count(Signal::BMP280_ALTITUDE)) {
		signals[Signal::BMP280_ALTITUDE] = mbl_mw_baro_bmp280_get_altitude_data_signal(board);
	}

	if (toggleSwitch != nullptr) {
		if (toggleSwitch->IsOn) {
			mbl_mw_datasignal_subscribe(signals[Signal::BMP280_ALTITUDE], baroAlt_handler);
			mbl_mw_datasignal_subscribe(signals[Signal::BMP280_PRESSURE], baroPre_handler);
			mbl_mw_baro_bmp280_start(board);
		} else {
			mbl_mw_baro_bmp280_stop(board);
			mbl_mw_datasignal_unsubscribe(signals[Signal::BMP280_ALTITUDE]);
			mbl_mw_datasignal_unsubscribe(signals[Signal::BMP280_PRESSURE]);
		}
	}
}

void DeviceInfo::toggleAmbientLightSampling(Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e) {
	ToggleSwitch^ toggleSwitch = dynamic_cast<ToggleSwitch^>(sender);
	if (!signals.count(Signal::AMBIENT_LIGHT)) {
		signals[Signal::AMBIENT_LIGHT] = mbl_mw_als_ltr329_get_illuminance_data_signal(board);
	}

	if (toggleSwitch != nullptr) {
		if (toggleSwitch->IsOn) {
			mbl_mw_datasignal_subscribe(signals[Signal::AMBIENT_LIGHT], amb_handler);
			mbl_mw_als_ltr329_start(board);
		} else {
			mbl_mw_als_ltr329_stop(board);
			mbl_mw_datasignal_unsubscribe(signals[Signal::AMBIENT_LIGHT]);
		}
	}
}

void DeviceInfo::toggleGyroSampling(Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e) {
	ToggleSwitch^ toggleSwitch = dynamic_cast<ToggleSwitch^>(sender);
	if (!signals.count(Signal::GYRO)) {
		signals[Signal::GYRO] = mbl_mw_gyro_bmi160_get_rotation_data_signal(board);
	}

	if (toggleSwitch != nullptr) {
		if (toggleSwitch->IsOn) {
			mbl_mw_datasignal_subscribe(signals[Signal::GYRO], gyro_handler);
			mbl_mw_gyro_bmi160_set_odr(board, MblMwGyroBmi160Odr::MBL_MW_GYRO_BMI160_ODR_25HZ);
			mbl_mw_gyro_bmi160_set_range(board, MblMwGyroBmi160Range::MBL_MW_GYRO_BMI160_FSR_500DPS);
			mbl_mw_gyro_bmi160_write_config(board);
			mbl_mw_gyro_bmi160_enable_rotation_sampling(board);
			mbl_mw_gyro_bmi160_start(board);
		} else {
			mbl_mw_gyro_bmi160_stop(board);
			mbl_mw_gyro_bmi160_disable_rotation_sampling(board);
			mbl_mw_datasignal_unsubscribe(signals[Signal::GYRO]);
		}
	}
}

void DeviceInfo::toggleSwitchSampling(Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e) {
	ToggleSwitch^ toggleSwitch = dynamic_cast<ToggleSwitch^>(sender);

	if (!signals.count(Signal::SWITCH)) {
		signals[Signal::SWITCH] = mbl_mw_switch_get_state_data_signal(board);
	}

	if (toggleSwitch != nullptr) {
		if (toggleSwitch->IsOn) {
			mbl_mw_datasignal_subscribe(signals[Signal::SWITCH], switch_handler);
		} else {
			mbl_mw_datasignal_unsubscribe(signals[Signal::SWITCH]);
		}
	}
}

void DeviceInfo::ledRedOn(Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e) {
	MblMwLedPattern *pattern = new MblMwLedPattern();
	mbl_mw_led_load_preset_pattern(pattern, MblMwLedPreset::MBL_MW_LED_PRESET_SOLID);
	mbl_mw_led_write_pattern(board, pattern, MblMwLedColor::MBL_MW_LED_COLOR_RED);
	mbl_mw_led_play(board);
}

void DeviceInfo::ledGreenOn(Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e) {
	MblMwLedPattern *pattern = new MblMwLedPattern();
	mbl_mw_led_load_preset_pattern(pattern, MblMwLedPreset::MBL_MW_LED_PRESET_BLINK);
	mbl_mw_led_write_pattern(board, pattern, MblMwLedColor::MBL_MW_LED_COLOR_GREEN);
	mbl_mw_led_play(board);
}

void DeviceInfo::ledBlueOn(Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e) {
	MblMwLedPattern *pattern = new MblMwLedPattern();
	mbl_mw_led_load_preset_pattern(pattern, MblMwLedPreset::MBL_MW_LED_PRESET_PULSE);
	mbl_mw_led_write_pattern(board, pattern, MblMwLedColor::MBL_MW_LED_COLOR_BLUE);
	mbl_mw_led_play(board);
}

void DeviceInfo::ledOff(Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e) {
	mbl_mw_led_stop_and_clear(board);
}

void commands_recorded() {
	mbl_mw_timer_start(gpio_read_timer);
}

void timer_created(MblMwTimer* timer) {
	gpio_read_timer = timer;

	mbl_mw_event_record_commands(timer);
	// Read analog input from pin 0 when a timer event is fired
	mbl_mw_gpio_read_digital_input(board, 0);
	mbl_mw_event_end_record(timer, commands_recorded);
}

void setup_timer() {
	// Create a timer that runs indefinitely
	mbl_mw_timer_create_indefinite(board, 1000, 0, timer_created);
}

// toggle reading input from PIN0 using the timer
void DeviceInfo::toggleGPIO(Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e) {
	ToggleSwitch^ sw = dynamic_cast<ToggleSwitch^>(sender);

	if (sw != nullptr) {
		byte pin = 0;
		MblMwDataSignal *gpMsignal = mbl_mw_gpio_get_digital_input_data_signal(board, pin);
		if (sw->IsOn) {
			mbl_mw_gpio_set_pin_change_type(board, pin, MblMwGpioPinChangeType::MBL_MW_GPIO_PIN_CHANGE_TYPE_ANY);
			mbl_mw_gpio_clear_digital_output(board, pin);
			mbl_mw_gpio_set_pull_mode(board, pin, MblMwGpioPullMode::MBL_MW_GPIO_PULL_MODE_DOWN);
			mbl_mw_datasignal_subscribe(gpMsignal, gpio_handler);
			setup_timer();
		} else {
			mbl_mw_datasignal_unsubscribe(gpMsignal);
			mbl_mw_timer_stop(gpio_read_timer);
			mbl_mw_timer_remove(gpio_read_timer);

		}
	}
}


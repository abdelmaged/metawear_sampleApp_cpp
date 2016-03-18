//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "MainPage.xaml.h"
#include "DeviceInfo.xaml.h"

using namespace MW_CPP_UWP_SampleApp;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::Devices::Enumeration;
using namespace Windows::Devices::Bluetooth;
using namespace Windows::Devices::Bluetooth::GenericAttributeProfile;
using namespace Windows::UI::Core;
using namespace concurrency;
using namespace Platform;


MainPage::MainPage() {
	InitializeComponent();

	this->NavigationCacheMode = Windows::UI::Xaml::Navigation::NavigationCacheMode::Enabled;
}

void MainPage::OnNavigatedTo(NavigationEventArgs^ e) {
	GetPairedBluetoothDevices();
}

void MainPage::refreshDevices(Object^ sender, RoutedEventArgs^ e) {
	GetPairedBluetoothDevices();
}

void MainPage::GetPairedBluetoothDevices() {
	create_task(DeviceInformation::FindAllAsync(BluetoothLEDevice::GetDeviceSelector()))
		.then([this](DeviceInformationCollection^ deviceCollection) {

		pairedDevicesListView->Items->Clear();
		if (deviceCollection != nullptr) {
			for each(DeviceInformation^ bltInfo in deviceCollection) {
				create_task(BluetoothLEDevice::FromIdAsync(bltInfo->Id))
					.then([this](BluetoothLEDevice^ bleDevice) {
					if (bleDevice != nullptr) {
						pairedDevicesListView->Items->Append(bleDevice);
					}
				});
			}
		}
	});
}

void MainPage::SelectedBtleDevice(Object^ sender, SelectionChangedEventArgs^ e) {
	// Ignore unselection event fire.
	if (e->AddedItems->Size == 0) return;

	//Get the data object that represents the current selected item
	BluetoothLEDevice^ myobject = dynamic_cast<BluetoothLEDevice^>((dynamic_cast<ListView^>(sender))->SelectedItem);

	//Checks whether that it is not null 
	if (myobject != nullptr && myobject->ConnectionStatus == BluetoothConnectionStatus::Connected) {
		this->Frame->Navigate(Windows::UI::Xaml::Interop::TypeName(DeviceInfo::typeid), myobject);
	} else {
		GetPairedBluetoothDevices();
	}
}

//
// DeviceInfo.xaml.h
// Declaration of the DeviceInfo class
//

#pragma once

#include "pch.h"
#include "DeviceInfo.g.h"
#include "metawear/core/connection.h"
#include "metawear/core/metawearboard.h"
#include <guiddef.h>
#include <unordered_map>
#include <string>

using namespace std;
using namespace Windows::Devices::Bluetooth;
using namespace Windows::Devices::Bluetooth::GenericAttributeProfile;
using namespace Platform;


namespace MW_CPP_UWP_SampleApp
{
	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class DeviceInfo sealed
	{
		
	private:
		
	public:
		
		DeviceInfo();
		
		void startMotor(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void startBuzzer(Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void toggleAccelerationSampling(Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void toggleBarometerSampling(Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void toggleAmbientLightSampling(Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void toggleGyroSampling(Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void toggleSwitchSampling(Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void ledRedOn(Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void ledGreenOn(Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void ledBlueOn(Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void ledOff(Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void toggleGPIO(Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

		void updateOutputListView(String^ str) {
			this->outputListView->Items->Append(str);
		}

		void toggleSensors(bool sw) {
			this->swAcc->IsOn = sw;
			this->swGyro->IsOn = sw;
		}



		virtual void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) override;

	};
}

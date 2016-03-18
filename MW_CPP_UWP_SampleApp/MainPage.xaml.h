//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once
#include "pch.h"
#include "MainPage.g.h"

using namespace concurrency;
using namespace Platform;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;


namespace MW_CPP_UWP_SampleApp
{
	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	public ref class MainPage sealed
	{
	public:
		MainPage();

		virtual void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) override;
	private:
		void refreshDevices(Object^ sender, RoutedEventArgs^ e);

		void GetPairedBluetoothDevices();

		void SelectedBtleDevice(Object^ sender, SelectionChangedEventArgs^ e);
	};
}

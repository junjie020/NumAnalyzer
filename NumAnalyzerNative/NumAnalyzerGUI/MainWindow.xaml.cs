using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

using System.Runtime.InteropServices;
using System.Text.RegularExpressions;
using Microsoft.WindowsAPICodePack.Dialogs;

using Json = Newtonsoft.Json;
using Linq = Newtonsoft.Json.Linq;
using Debug = System.Diagnostics.Debug;

namespace NumAnalyzerGUI
{
	/// <summary>
	/// Interaction logic for MainWindow.xaml
	/// </summary>
	public partial class MainWindow : Window
	{
		public MainWindow()
		{
			InitializeComponent();
		}

		private void MyClick(object obj, RoutedEventArgs e)
		{			
			//System.Console.WriteLine("object name :{0}, event string :{1}", obj.ToString(), e.ToString());
		}

		private void Update_Click(object sender, RoutedEventArgs e)
		{
			//System.Console.WriteLine("url text box string : " + Update.Name);
		}

		//[StructLayout(LayoutKind.Sequential)]
		//public struct AnalyzeResultInfo
		//{
		//	public float fff;
		//	public int outputInfoCount;			
		//}


		[DllImport("NumanalyzerNative.dll", CallingConvention = CallingConvention.Cdecl)]
		private static extern int fnNumanalyzerNative(string filePath, StringBuilder output, int outputBufferSize);

		//[DllImport("NumanalyzerNative.dll", CallingConvention = CallingConvention.Cdecl)]
		//private static extern int Add(int a, int c);

		class OutputResultContent
		{

		}

		private TabItem GetItemByName(string name)
		{
			foreach (TabItem tab in TabC.Items)
			{
				if (tab.Name == name)
					return tab;
			}

			return null;
		}

		private uint[] ConvertStringToUIntList(string str)
		{
			if (str == null)
				return new uint[0];

			string[] strNumbers = str.Split(',');
			uint[] numbers = new uint[strNumbers.Length];

			for (int ii = 0; ii < numbers.Length; ++ii)
			{
				numbers[ii] = UInt32.Parse(strNumbers[ii]);
			}

			return numbers;
		}


		private void PrintInfoToTabs(string jsonString)
		{
			Linq.JObject jObj = Linq.JObject.Parse(jsonString);

			string[] analyzeTypeName = { "BigSmall", "OddEven", "NumBigSmall", "NumOddEven" };

			Linq.JObject continueJObj = jObj["BigSmall"]["Continue"] as Linq.JObject;
			
			Debug.Assert(continueJObj != null);

			Linq.JArray columns = continueJObj["Column"] as Linq.JArray;
			Debug.Assert(columns != null && columns.Count == 10);

			string continueTextInfo;
			
			for (uint iCol = 0; iCol < columns.Count; ++iCol)
			{
				Linq.JArray infoArrayJObj = columns[iCol] as Linq.JArray;
				Debug.Assert(infoArrayJObj != null);

				for (uint iInfo = 0; iInfo < infoArrayJObj.Count; ++iInfo)
				{
					Linq.JObject infoJObj = infoArrayJObj[iInfo] as Linq.JObject;

					continueTextInfo = (infoJObj["Counter"]).ToString();
				}
			
			}

			

		}
	

		private void AnalyzeResult(object sender, RoutedEventArgs e)
		{
			//System.Console.WriteLine("tab index {0}", TabC.SelectedItem.GetType().ToString());

			if (FileTextBox.Text.Length != 0)
			{
				const int bufferSize = 1024 * 1024;
				StringBuilder outputResults = new StringBuilder(bufferSize);
				fnNumanalyzerNative(FileTextBox.Text, outputResults, bufferSize);

				PrintInfoToTabs(outputResults.ToString());



			}
			//else
			//{
			//	// test
			//	//string json = @"{
			//	//'CPU': 'Intel',
			//	//'PSU': '500W',
			//	//'Drives': [
			//	//'DVD read/writer'
			//	///*(broken)*/,
			//	//'500 gigabyte hard drive',
			//	//'200 gigabype hard drive'
			//	//]
			//	//}";
			//	////System.IO.File 

			//	System.IO.StreamReader rw = System.IO.File.OpenText(@"G:\github\NumAnalyzer\NumAnalyzerNative\NumAnalyzerConsoleTest\jsontest.json");
			//	string fileContent = rw.ReadToEnd();
			//	Newtonsoft.Json.Linq.JObject jObj = Newtonsoft.Json.Linq.JObject.Parse(fileContent);

			//	Newtonsoft.Json.Linq.JArray arr = (Newtonsoft.Json.Linq.JArray)jObj["BigSmall"]["Continue"]["Column"];

				

			//}
		}

		private void BrownFile(object sender, RoutedEventArgs e)
		{
			CommonOpenFileDialog dlg = new CommonOpenFileDialog();

			if (dlg.ShowDialog() == CommonFileDialogResult.Ok)
			{
				FileTextBox.Text = dlg.FileName;
			}
		}
	}
}

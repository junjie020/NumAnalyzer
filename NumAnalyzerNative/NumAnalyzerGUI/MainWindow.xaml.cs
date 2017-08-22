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
		//[DllImport("NumanalyzerNative.dll", CallingConvention = CallingConvention.Cdecl)]
		//private static extern int NumanalyzerNativeFromPath(string filePath, StringBuilder output, int outputBufferSize);

		//[DllImport("NumanalyzerNative.dll", CallingConvention = CallingConvention.Cdecl)]
		//private static extern int NumanalyzerNativeFromURL(string URL, StringBuilder output, int outputBufferSize);

		[DllImport("NumanalyzerNative.dll", CallingConvention = CallingConvention.Cdecl)]
		private static extern int NumAnalyzeNative(string URL, StringBuilder output, int outputBufferSize, bool isURL);

		[DllImport("NumanalyzerNative.dll", CallingConvention = CallingConvention.Cdecl)]
		private static extern int InitNative();

		private bool needDisableURL = false;
		public MainWindow()
		{
			int result = InitNative();
			if (result == 1)
				needDisableURL = true;

			InitializeComponent();
		}

		private void Update_Click(object sender, RoutedEventArgs e)
		{
			AnalyzeResult(URLTextBox.Text, true);
		}

		//[StructLayout(LayoutKind.Sequential)]
		//public struct AnalyzeResultInfo
		//{
		//	public float fff;
		//	public int outputInfoCount;			
		//}




		//[DllImport("NumanalyzerNative.dll", CallingConvention = CallingConvention.Cdecl)]
		//private static extern int Add(int a, int c);

		static private readonly string LineSeparator_String		= "----------------------------------------";
		static private readonly string MemberSeparator_String	= " | ";
		static private readonly string NewLine_String			= "\n";


		private string BuildRecordInfo(Linq.JObject recordJObj, string prefixInfo)
		{
			string outputResult = prefixInfo;

			Linq.JArray columns = recordJObj["Column"] as Linq.JArray;
			Debug.Assert(columns != null && columns.Count == 10);			

			System.Collections.Generic.SortedDictionary<int, int> counterTimesMap = new SortedDictionary<int, int>();

			for (int iCol = 0; iCol < columns.Count; ++iCol)
			{
				Linq.JArray infoArrayJObj = columns[iCol] as Linq.JArray;
				Debug.Assert(infoArrayJObj != null);

				for (int iInfo = 0; iInfo < infoArrayJObj.Count; ++iInfo)
				{
					Linq.JObject infoJObj = infoArrayJObj[iInfo] as Linq.JObject;
					Debug.Assert(infoJObj != null);
					Linq.JArray originInfoJArray = infoJObj["OriginInfo"] as Linq.JArray;
					Debug.Assert(originInfoJArray != null);

					int counter = (int)infoJObj["Counter"];

					if (!counterTimesMap.ContainsKey(counter))
					{
						counterTimesMap.Add(counter, originInfoJArray.Count);
					}
					else
					{
						counterTimesMap[counter] += originInfoJArray.Count;
					}
				}
			}

			int numberIndex = 0;
			foreach (var item in counterTimesMap)
			{
				outputResult += string.Format("{0}({1})", item.Key, item.Value);
				outputResult += (numberIndex % 3 == 0) && (numberIndex != 0) ? NewLine_String : MemberSeparator_String;
				++numberIndex;
			}
		
			return outputResult;
		}

		private string BuildAnalyzeTypeInfo(Linq.JObject analyzeJObj)
		{
			string outputResult = "";

			var continueJObj = analyzeJObj["Continue"] as Linq.JObject;
			Debug.Assert(continueJObj != null);

			outputResult += BuildRecordInfo(continueJObj, "连续情况：" + NewLine_String);

			outputResult += NewLine_String + LineSeparator_String + NewLine_String;

			var stepJObj = analyzeJObj["Step"] as Linq.JObject;
			Debug.Assert(stepJObj != null);

			outputResult += BuildRecordInfo(stepJObj, "隔断情况：" + NewLine_String);

			return outputResult;
		}

		private void PrintInfoToTabs(string jsonString)
		{
			Linq.JObject jObj = Linq.JObject.Parse(jsonString);

			if (jObj == null)
			{
				System.Console.WriteLine("parse json result failed!");
				return;
			}
				
			Linq.JObject bigSmallAnalyzeResultJObj = jObj["BigSmall"] as Linq.JObject;
			Debug.Assert(bigSmallAnalyzeResultJObj != null);

			BigSmallTabContent.Text = BuildAnalyzeTypeInfo(bigSmallAnalyzeResultJObj);


			Linq.JObject oddEvenAnalyzeResultJObj = jObj["OddEven"] as Linq.JObject;
			Debug.Assert(oddEvenAnalyzeResultJObj != null);

			OddEvenTabContent.Text = BuildAnalyzeTypeInfo(oddEvenAnalyzeResultJObj);

			Linq.JObject numBigSmallAnalyzeResultJObj = jObj["NumBigSmall"] as Linq.JObject;
			Debug.Assert(numBigSmallAnalyzeResultJObj != null);

			NumBigSmallTabContent.Text = BuildAnalyzeTypeInfo(numBigSmallAnalyzeResultJObj);

			Linq.JObject numOddEvenAnalyzeResultJObj = jObj["NumOddEven"] as Linq.JObject;
			Debug.Assert(numOddEvenAnalyzeResultJObj != null);

			NumOddEvenTabContent.Text = BuildAnalyzeTypeInfo(numOddEvenAnalyzeResultJObj);
		}

		private void AnalyzeResult(string content, bool isURL)
		{
			const int bufferSize = 1024 * 1024;
			StringBuilder outputResults = new StringBuilder(bufferSize);

			NumAnalyzeNative(content, outputResults, bufferSize, isURL);
			
			PrintInfoToTabs(outputResults.ToString());
		}
	

		private void AnalyzeResult(object sender, RoutedEventArgs e)
		{
			if (FileTextBox.Text.Length != 0)
			{
				AnalyzeResult(FileTextBox.Text, false);
			}
		}

		private void BrownFile(object sender, RoutedEventArgs e)
		{
			CommonOpenFileDialog dlg = new CommonOpenFileDialog();

			if (dlg.ShowDialog() == CommonFileDialogResult.Ok)
			{
				FileTextBox.Text = dlg.FileName;
			}
		}

		private void UsingURLAsInput(object sender, RoutedEventArgs e)
		{
			bool isCheck = (bool)URLCheck.IsChecked;
			FileTextBox.IsEnabled = !isCheck;
			Brown.IsEnabled = !isCheck;

			URLTextBox.IsEnabled = isCheck;
		}
	}
}

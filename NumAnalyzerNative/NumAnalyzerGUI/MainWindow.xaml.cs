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

		[DllImport("NumanalyzerNative.dll", CallingConvention = CallingConvention.Cdecl)]
		private static extern void URLPageToReadNative(int page);		

		private bool needDisableURL = false;
		public MainWindow()
		{
			int result = InitNative();
			if (result == 1)
				needDisableURL = true;

			InitializeComponent();

			if(needDisableURL)
			{
				URLCheck.IsEnabled = false;
				PageToRead.IsEnabled = false;

				FileTextBox.IsEnabled = true;
				Brown.IsEnabled = true;

				System.Windows.MessageBox.Show("确认网路是否有连接，网址模式不可用", "警告", MessageBoxButton.OK);
			}

			URLPageToReadNative(System.Math.Max(1, Int32.Parse(PageToRead.Text)));
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
			Debug.Assert(analyzeJObj != null);
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

		private string BuildOriginNumbersContent(Linq.JArray originNumbersObj)
		{
			Debug.Assert(originNumbersObj != null);
			string content = "";
			for (int iRow = 0; iRow < originNumbersObj.Count; ++iRow)
			{
				Linq.JArray lineArray = originNumbersObj[iRow] as Linq.JArray;
				Debug.Assert(lineArray != null);

				Debug.Assert(lineArray.Count == 10);

				string lineContent = "[";
				for (int iCol = 0; iCol < lineArray.Count; ++iCol)
				{
					lineContent += lineArray[iCol].ToString();

					if (iCol != lineArray.Count - 1)
					{
						lineContent += ", ";
					}					
				}

				lineContent += "]\n";
				content += lineContent;
			}

			return content;
		}

		private void PrintInfoToTabs(string jsonString)
		{
			if (jsonString == "")
				return;

			Linq.JObject jObj = Linq.JObject.Parse(jsonString);

			if (jObj == null)
			{
				System.Console.WriteLine("parse json result failed!");
				return;
			}
				
			Linq.JObject bigSmallAnalyzeResultJObj = jObj["BigSmall"] as Linq.JObject;								
			BigSmallTabContent.Text = BuildAnalyzeTypeInfo(bigSmallAnalyzeResultJObj);


			Linq.JObject oddEvenAnalyzeResultJObj = jObj["OddEven"] as Linq.JObject;
			OddEvenTabContent.Text = BuildAnalyzeTypeInfo(oddEvenAnalyzeResultJObj);

			Linq.JObject numBigSmallAnalyzeResultJObj = jObj["NumBigSmall"] as Linq.JObject;
			NumBigSmallTabContent.Text = BuildAnalyzeTypeInfo(numBigSmallAnalyzeResultJObj);

			Linq.JObject numOddEvenAnalyzeResultJObj = jObj["NumOddEven"] as Linq.JObject;
			NumOddEvenTabContent.Text = BuildAnalyzeTypeInfo(numOddEvenAnalyzeResultJObj);

			Linq.JArray originNumbersObj = jObj["OriginNumbers"] as Linq.JArray;
			OriginNumbersTabContent.Text = BuildOriginNumbersContent(originNumbersObj);




		}

		private void AnalyzeResult(string content, bool isURL)
		{
			URLPageToReadNative(System.Math.Max(1, Int32.Parse(PageToRead.Text)));

			const int bufferSize = 1024 * 1024;
			StringBuilder outputResults = new StringBuilder(bufferSize);

			NumAnalyzeNative(content, outputResults, bufferSize, isURL);
			
			PrintInfoToTabs(outputResults.ToString());
		}
	

		private void AnalyzeResult(object sender, RoutedEventArgs e)
		{
			bool isURL = (bool)URLCheck.IsChecked;
			string content = isURL ? URLTextBox.Text : FileTextBox.Text;
			if (content.Length != 0)
			{
				AnalyzeResult(content, isURL);
			}
			else
			{
				System.Windows.MessageBox.Show("输入数据为空","警告", MessageBoxButton.OK);
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
			PageToRead.IsEnabled = isCheck;
		}
	}
}

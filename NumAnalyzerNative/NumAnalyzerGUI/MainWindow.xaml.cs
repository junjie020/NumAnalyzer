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
		private static extern int fnNumanalyzerNative(string filePath, StringBuilder output);

		//[DllImport("NumanalyzerNative.dll", CallingConvention = CallingConvention.Cdecl)]
		//private static extern int Add(int a, int c);

		class OutputResultContent
		{
			public struct AnalyzeResult
			{
				public string big;
				public string small;
				public string odd;
				public string even; 
			}

			public enum AnalyzeResultType : int
			{
				ART_Continue = 0,
				ART_Step,
				ART_NumContinue,
				ART_NumStep,
				ART_Count,
			}

			public AnalyzeResult[] results;

			public OutputResultContent()
			{
				results = new AnalyzeResult[(int)(AnalyzeResultType.ART_Count)];
			}
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

		private void FormatString(uint[] first, uint[] second, string firstName, string secondName, ref string formatStr)
		{
			uint count = (uint)Math.Min(first.Length, second.Length);

			for (uint ii = 0; ii < count; ++ii)
			{
				for (uint idx0 = 0; idx0 < first[ii]; ++idx0)
				{
					formatStr += firstName + ", ";
				}

				formatStr = formatStr.TrimEnd();
				formatStr = formatStr.Remove(formatStr.Length - 1);
				formatStr += " | ";

				for (uint idx1 = 0; idx1 < second[ii]; ++idx1)
				{
					formatStr += secondName + ", ";
				}

				formatStr = formatStr.TrimEnd();
				formatStr = formatStr.Remove(formatStr.Length - 1);
				formatStr += " | ";
			}

			if (first.Length > second.Length)
			{
				formatStr += firstName;
			}
			else if (second.Length > first.Length)
			{
				formatStr += secondName;
			}
			else
			{
				if (0 != formatStr.Length)
				{
					formatStr = formatStr.Trim();
					System.Diagnostics.Debug.Assert(formatStr[formatStr.Length - 1] == '|');
					formatStr = formatStr.Remove(formatStr.Length - 1);
					formatStr = formatStr.Trim();
				}
			}

			if (0 != formatStr.Length)
				formatStr += '\n';
		}

		private string FormatOutput(ref OutputResultContent.AnalyzeResult result)
		{
			string formatStr = "";
			uint[] bigNumbers	= ConvertStringToUIntList(result.big);
			uint[] smallNumbers = ConvertStringToUIntList(result.small);

			FormatString(bigNumbers, smallNumbers, "大", "小", ref formatStr);

			uint[] oddNumbers	= ConvertStringToUIntList(result.odd);
			uint[] evenNumbers	= ConvertStringToUIntList(result.even);

			FormatString(oddNumbers, evenNumbers, "单", "双", ref formatStr);

			return formatStr;
		}

		private void AnalyzeResult(object sender, RoutedEventArgs e)
		{
			//System.Console.WriteLine("tab index {0}", TabC.SelectedItem.GetType().ToString());

			if (FileTextBox.Text.Length != 0)
			{
				StringBuilder outputResults = new StringBuilder(1024*1024);
				fnNumanalyzerNative(FileTextBox.Text, outputResults);

				OutputResultContent outputContent = new OutputResultContent();

				ExtractOutputResult(outputResults.ToString(), ref outputContent);

				List<TextBox> tbs = new List<TextBox> { ContinueTabContent, StepTabContent, NumContinueTabContent, NumStepTabContent};

				for (int ii = 0; ii < outputContent.results.Length; ++ii)
				{
					tbs[ii].Text = FormatOutput(ref outputContent.results[ii]);
				}
			}
		}

		private OutputResultContent.AnalyzeResultType FindResultType(string name)
		{
			switch (name)
			{
				case "ContinueAnalyzer":
					return OutputResultContent.AnalyzeResultType.ART_Continue;
				case "StepAnalyzer":
					return OutputResultContent.AnalyzeResultType.ART_Step;
				case "NumContinueAnalyzer":
					return OutputResultContent.AnalyzeResultType.ART_NumContinue;
				case "NumStepAnalyzer":
					return OutputResultContent.AnalyzeResultType.ART_NumStep;
				default:
					return OutputResultContent.AnalyzeResultType.ART_Count;
			}
		}

		private string[] GetLines(string output)
		{
			string[] results = output.Split('\n');

			results = results.Where(ss => ss.Length != 0).ToArray();
			return results;
		}

		private string ExtractAnalyzerName(string line)
		{
			const string namePrefix = "Name : ";
			int idx = line.IndexOf(namePrefix);
			if (idx == -1)
				return "";

			return line.Substring(namePrefix.Length).Trim();
		}

		private void ExtractOutputResult(string nativeOutputResult, ref OutputResultContent output)
		{
			string[] lines = GetLines(nativeOutputResult);

			if (lines.Length == 0)
				return;

			const int AnalyzerResultStep = 5;
			System.Diagnostics.Debug.Assert(((lines.Length % AnalyzerResultStep)) == 0);

			int analyzerCount = lines.Length / AnalyzerResultStep;

			if (analyzerCount == 0)
				return ;

			for (int ii = 0; ii < analyzerCount; ++ii)
			{
				int analyzerIdx = ii * AnalyzerResultStep;
				OutputResultContent.AnalyzeResultType resultType = FindResultType(ExtractAnalyzerName(lines[analyzerIdx]));
				if (resultType == OutputResultContent.AnalyzeResultType.ART_Count)
					continue;

				ref OutputResultContent.AnalyzeResult result = ref output.results[(int)resultType];

				for (int contentIdx = 1; contentIdx < AnalyzerResultStep; ++contentIdx)
				{
					string line = lines[contentIdx + analyzerIdx];
					System.Diagnostics.Debug.Assert(line[line.Length - 1] == ',');
				
					int lineContentSplitIdx = line.IndexOf(":");
					string content = line.Substring(lineContentSplitIdx + 1);
					content = content.Trim();

					if (content[content.Length - 1] == ',')
					{
						content = content.Remove(content.Length - 1);
					}

					string name = line.Substring(0, lineContentSplitIdx);
					name = name.Trim();

					if (lineContentSplitIdx != -1)
					{ 
						switch (name)
						{
							case "Big":		result.big		= content;	break;
							case "Small":	result.small	= content;	break;
							case "Odd":		result.odd		= content;	break;
							case "Even":	result.even		= content;	break;
							default:		System.Diagnostics.Debug.Assert(false);	break;
						}
					}					
				}

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
	}
}

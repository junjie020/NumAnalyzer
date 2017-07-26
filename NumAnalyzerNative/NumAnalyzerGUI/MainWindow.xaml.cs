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


		private int mCounter;

		private void MyClick(object obj, RoutedEventArgs e)
		{
			++mCounter;
			System.Console.WriteLine("object name :{0}, event string :{1}", obj.ToString(), e.ToString());
		}

		private void Update_Click(object sender, RoutedEventArgs e)
		{
			OutputWin.Text = URLTextBox.Text;


			System.Console.WriteLine("url text box string : " + Update.Name);
		}


		[DllImport("NumanalyzerNative.dll", CallingConvention = CallingConvention.Cdecl)]
		private static extern int fnNumanalyzerNative(char []tablename, char []output);

		//[DllImport("NumanalyzerNative.dll", CallingConvention = CallingConvention.Cdecl)]
		//private static extern int Add(int a, int c);

		private void AnalyzeResult(object sender, RoutedEventArgs e)
		{
			if (FileTextBox.Text.Length != 0)
			{
				char[] bs = new char[1024];
				FileTextBox.Text.CopyTo(0, bs, 0, System.Math.Min(512, FileTextBox.Text.Length));
				char[] resultOutput = new char[1024 * 100];
				fnNumanalyzerNative(bs, resultOutput);
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

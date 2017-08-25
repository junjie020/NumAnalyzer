del .\package /f /y /s
mkdir .\package
set targetPath=.\package\NumAnalyzerNative
mkdir %targetPath%

copy /y Release\libcurl.dll %targetPath%\libcurl.dll 
copy /y Release\Microsoft.WindowsAPICodePack.dll %targetPath%\Microsoft.WindowsAPICodePack.dll 
copy /y Release\Microsoft.WindowsAPICodePack.Shell.dll %targetPath%\Microsoft.WindowsAPICodePack.Shell.dll 
copy /y Release\Microsoft.WindowsAPICodePack.Shell.xml %targetPath%\Microsoft.WindowsAPICodePack.Shell.xml 
copy /y Release\Microsoft.WindowsAPICodePack.xml %targetPath%\Microsoft.WindowsAPICodePack.xml 
copy /y Release\Newtonsoft.Json.dll %targetPath%\Newtonsoft.Json.dll 
copy /y Release\Newtonsoft.Json.xml %targetPath%\Newtonsoft.Json.xml 
copy /y Release\NumAnalyzerGUI.exe %targetPath%\NumAnalyzerGUI.exe 
copy /y Release\NumAnalyzerGUI.exe.config %targetPath%\NumAnalyzerGUI.exe.config 
copy /y Release\NumAnalyzerGUI.exe.manifest %targetPath%\NumAnalyzerGUI.exe.manifest 
copy /y Release\NumAnalyzerGUI.pdb %targetPath%\NumAnalyzerGUI.pdb 
copy /y Release\NumanalyzerNative.dll %targetPath%\NumanalyzerNative.dll 
copy /y Release\NumanalyzerNative.exp %targetPath%\NumanalyzerNative.exp 
copy /y Release\NumanalyzerNative.pdb %targetPath%\NumanalyzerNative.pdb 

set resPath=.\package\Resources
mkdir %resPath%

copy /y .\Resources\*.* %resPath%\*.*

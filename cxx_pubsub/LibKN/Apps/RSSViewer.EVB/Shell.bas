Attribute VB_Name = "basShell"
Option Explicit

Declare Function MyCreateProcess Lib "coredll" Alias "CreateProcessW" (ByVal lpApplicationName As String, ByVal lpCommandLine As String, ByVal lpProcessAttributes As Long, ByVal lpThreadAttributes As Long, ByVal bInheritHandles As Long, ByVal dwCreationFlags As Long, ByVal lpEnvironment As Long, ByVal lpCurrentDirectory As Long, ByVal lpStartupInfo As Long, ByVal lpProcessInformation As String) As Long
Declare Function MyGetLastError Lib "coredll" Alias "GetLastError" () As Long
Declare Function MyCloseHandle Lib "coredll" Alias "CloseHandle" (ByVal hObject As Long) As Long
Function Shell(ByVal Application As String, ByVal Parameters As String) As Long
  
  Dim lsPI As String, llResult As Long, lhProcess As Long, lhThread As Long
  
  lsPI = LongToBytes(0) & LongToBytes(0) & LongToBytes(0) & LongToBytes(0)
  llResult = MyCreateProcess(Application, Parameters, 0, 0, 0, 0, 0, 0, 0, lsPI)
  If llResult <> 0 Then
    lhThread = BytesToLong(MidB(lsPI, 5, 4))
    Call MyCloseHandle(lhThread)
    lhProcess = BytesToLong(MidB(lsPI, 1, 4))
    Call MyCloseHandle(lhProcess)
    llResult = 0
  Else
    llResult = MyGetLastError()
    If llResult = 0 Then llResult = -1
  End If
  Shell = llResult

End Function
Function LongToBytes(ByVal Value As Long) As String
  
  Dim lsHex As String, i As Integer
  
  lsHex = Right("00000000" & Hex(Value), 8)
  For i = 1 To 7 Step 2
    LongToBytes = ChrB(CInt("&H" & Mid(lsHex, i, 2))) & LongToBytes
  Next

End Function
Function BytesToLong(ByVal Value As String) As Long
  
  Dim lsHex As String, i As Integer
  
  For i = 1 To 4
    lsHex = Hex(AscB(MidB(Value, i, 1))) & lsHex
  Next
  BytesToLong = CLng("&H" & lsHex)

End Function

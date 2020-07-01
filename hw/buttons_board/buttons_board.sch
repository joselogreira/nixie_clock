EESchema Schematic File Version 4
LIBS:NC2_buttonsBoard-cache
EELAYER 26 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L NC2_buttonsBoard-rescue:BARREL_JACK CON1
U 1 1 57B06720
P 4350 2875
F 0 "CON1" H 4350 3125 50  0000 C CNN
F 1 "BARREL_JACK" H 4350 2675 50  0000 C CNN
F 2 "usrConnectors:CON_PWR_JACK_5.5x2mm" H 4350 2875 50  0001 C CNN
F 3 "" H 4350 2875 50  0000 C CNN
	1    4350 2875
	1    0    0    -1  
$EndComp
$Comp
L NC2_buttonsBoard-rescue:SWITCH_SPST SW3
U 1 1 57B068C9
P 6250 3975
F 0 "SW3" H 6250 3850 60  0000 C CNN
F 1 "MODE" H 6250 4125 60  0000 C CNN
F 2 "usrMisc:SW_SPST_THT_6mm" H 6350 3975 60  0001 C CNN
F 3 "" H 6350 3975 60  0000 C CNN
	1    6250 3975
	1    0    0    -1  
$EndComp
$Comp
L NC2_buttonsBoard-rescue:SWITCH_SPST SW2
U 1 1 57B06966
P 6250 3625
F 0 "SW2" H 6250 3500 60  0000 C CNN
F 1 "SET" H 6250 3775 60  0000 C CNN
F 2 "usrMisc:SW_SPST_THT_6mm" H 6350 3625 60  0001 C CNN
F 3 "" H 6350 3625 60  0000 C CNN
	1    6250 3625
	1    0    0    -1  
$EndComp
$Comp
L NC2_buttonsBoard-rescue:SWITCH_SPST SW1
U 1 1 57B069A4
P 6250 3250
F 0 "SW1" H 6250 3125 60  0000 C CNN
F 1 "UP" H 6250 3400 60  0000 C CNN
F 2 "usrMisc:SW_SPST_THT_6mm" H 6350 3250 60  0001 C CNN
F 3 "" H 6350 3250 60  0000 C CNN
	1    6250 3250
	1    0    0    -1  
$EndComp
$Comp
L NC2_buttonsBoard-rescue:HOLE X1
U 1 1 57B06B37
P 4700 4450
F 0 "X1" H 4700 4300 60  0000 C CNN
F 1 "HOLE" H 4700 4600 60  0000 C CNN
F 2 "Mounting_Holes:MountingHole_3-5mm" H 4700 4450 60  0001 C CNN
F 3 "" H 4700 4450 60  0000 C CNN
	1    4700 4450
	1    0    0    -1  
$EndComp
$Comp
L NC2_buttonsBoard-rescue:HOLE X2
U 1 1 57B06BA6
P 5150 4425
F 0 "X2" H 5150 4275 60  0000 C CNN
F 1 "HOLE" H 5150 4575 60  0000 C CNN
F 2 "Mounting_Holes:MountingHole_3-5mm" H 5150 4425 60  0001 C CNN
F 3 "" H 5150 4425 60  0000 C CNN
	1    5150 4425
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR01
U 1 1 57B09F49
P 4775 3075
F 0 "#PWR01" H 4775 2825 50  0001 C CNN
F 1 "GND" H 4775 2925 50  0001 C CNN
F 2 "" H 4775 3075 50  0000 C CNN
F 3 "" H 4775 3075 50  0000 C CNN
	1    4775 3075
	1    0    0    -1  
$EndComp
Wire Wire Line
	4650 2875 4775 2875
Wire Wire Line
	4775 2875 4775 2975
Wire Wire Line
	4650 2975 4775 2975
Connection ~ 4775 2975
Wire Wire Line
	4650 2775 5250 2775
Wire Wire Line
	5250 2775 5250 2600
Wire Wire Line
	5250 2600 5850 2600
Wire Wire Line
	5850 2600 5850 2775
Wire Wire Line
	5850 2775 5800 2775
Connection ~ 5250 2775
$Comp
L power:GND #PWR02
U 1 1 57B0A266
P 5050 2900
F 0 "#PWR02" H 5050 2650 50  0001 C CNN
F 1 "GND" H 5050 2750 50  0001 C CNN
F 2 "" H 5050 2900 50  0000 C CNN
F 3 "" H 5050 2900 50  0000 C CNN
	1    5050 2900
	1    0    0    -1  
$EndComp
Wire Wire Line
	5050 2900 5050 2875
Wire Wire Line
	5050 2875 5300 2875
$Comp
L power:GND #PWR03
U 1 1 57B0A320
P 6125 2925
F 0 "#PWR03" H 6125 2675 50  0001 C CNN
F 1 "GND" H 6125 2775 50  0001 C CNN
F 2 "" H 6125 2925 50  0000 C CNN
F 3 "" H 6125 2925 50  0000 C CNN
	1    6125 2925
	1    0    0    -1  
$EndComp
Wire Wire Line
	6125 2875 6125 2925
Wire Wire Line
	5800 2875 5850 2875
Wire Wire Line
	5800 2975 5850 2975
Wire Wire Line
	5850 2975 5850 2875
Connection ~ 5850 2875
Wire Wire Line
	6000 3250 5950 3250
Wire Wire Line
	5950 3075 5950 3250
Wire Wire Line
	5950 3075 5800 3075
Wire Wire Line
	5950 3325 6000 3325
Connection ~ 5950 3250
Wire Wire Line
	5250 3075 5250 3625
Wire Wire Line
	5250 3075 5300 3075
Wire Wire Line
	5300 2975 5200 2975
Wire Wire Line
	5200 2975 5200 3975
$Comp
L power:GND #PWR04
U 1 1 57B0B013
P 6600 3350
F 0 "#PWR04" H 6600 3100 50  0001 C CNN
F 1 "GND" H 6600 3200 50  0001 C CNN
F 2 "" H 6600 3350 50  0000 C CNN
F 3 "" H 6600 3350 50  0000 C CNN
	1    6600 3350
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR05
U 1 1 57B0B039
P 6600 3725
F 0 "#PWR05" H 6600 3475 50  0001 C CNN
F 1 "GND" H 6600 3575 50  0001 C CNN
F 2 "" H 6600 3725 50  0000 C CNN
F 3 "" H 6600 3725 50  0000 C CNN
	1    6600 3725
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR06
U 1 1 57B0B05F
P 6600 4075
F 0 "#PWR06" H 6600 3825 50  0001 C CNN
F 1 "GND" H 6600 3925 50  0001 C CNN
F 2 "" H 6600 4075 50  0000 C CNN
F 3 "" H 6600 4075 50  0000 C CNN
	1    6600 4075
	1    0    0    -1  
$EndComp
Wire Wire Line
	6600 4075 6600 4050
Wire Wire Line
	6600 3975 6500 3975
Wire Wire Line
	6500 4050 6600 4050
Connection ~ 6600 4050
Wire Wire Line
	6600 3725 6600 3700
Wire Wire Line
	6600 3625 6500 3625
Wire Wire Line
	6500 3700 6600 3700
Connection ~ 6600 3700
Wire Wire Line
	6600 3350 6600 3325
Wire Wire Line
	6600 3250 6500 3250
Wire Wire Line
	6500 3325 6600 3325
Connection ~ 6600 3325
Wire Wire Line
	5250 3625 5950 3625
Wire Wire Line
	6000 3700 5950 3700
Wire Wire Line
	5950 3700 5950 3625
Connection ~ 5950 3625
Wire Wire Line
	5200 3975 5950 3975
Wire Wire Line
	6000 4050 5950 4050
Wire Wire Line
	5950 4050 5950 3975
Connection ~ 5950 3975
$Comp
L NC2_buttonsBoard-rescue:CONN_02X05 P1
U 1 1 59ED08E3
P 5550 2975
F 0 "P1" H 5550 3275 50  0000 C CNN
F 1 "CONN_02X05" H 5550 2675 50  0001 C CNN
F 2 "Pin_Headers:Pin_Header_Angled_2x05" H 5550 1775 50  0001 C CNN
F 3 "" H 5550 1775 50  0000 C CNN
	1    5550 2975
	1    0    0    -1  
$EndComp
NoConn ~ 5300 3175
NoConn ~ 5800 3175
Wire Wire Line
	4775 2975 4775 3075
Wire Wire Line
	5250 2775 5300 2775
Wire Wire Line
	5850 2875 6125 2875
Wire Wire Line
	5950 3250 5950 3325
Wire Wire Line
	6600 4050 6600 3975
Wire Wire Line
	6600 3700 6600 3625
Wire Wire Line
	6600 3325 6600 3250
Wire Wire Line
	5950 3625 6000 3625
Wire Wire Line
	5950 3975 6000 3975
$EndSCHEMATC

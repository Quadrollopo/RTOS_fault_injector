freeRTOS è compilabile tramite cmake, per compilarlo andare nella cartella FreeRTOS/Demo ed eseguire:
```
cmake . -B build
cd build
make
./freeRTOS
```

0x402000-0x427000 --> codice rx
0x432000-0x460000 --> heap rw
0x7ffffffde000-0x7ffffffff000 --> stack rw
0x431000-0x432000 --> variabili globali

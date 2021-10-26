freeRTOS è compilabile tramite cmake, per compilarlo su Linux andare nella cartella FreeRTOS/Demo ed eseguire:

```
cmake . -B build
cd build
make
./freeRTOS
```

# Robe da fare
 - [x] CMAKE
 - [ ] codare versione definitiva freeRTOS
 - [ ] injector
 - [ ] parallelizzare

# Indirizzi utili
Aggiungere indirizzi di var globali e strutture varie qui <br>

 | Indirizzo  |  Roba |   
|---|---|
| 0x402000-0x427000 | codice |
| 0x432000-0x460000 | heap |
|0x7ffffffde000-0x7ffffffff000 | stack |
|0x431000-0x432000 | variabili globali |

# Tipologie di errori
1. Nessun errore (o Masked) per cui l’output del sistema non subisce alcuna alterazione

2. un Silence Data Corruption (SDC), per cui il sistema continua a funzionare ma l’output
generato è diverso da quello atteso. Per questa opzione si cerchi di permettere all’utente
di impostare quale sia l’output atteso
3. un Delay, per cui il sistema produce l’output atteso ma con un ritardo rispetto al tempo
previsto. Per questa opzione, essendo il ritardo variabile, si studi come il FreeRTOS defi-
nisce le deadline e si associ questa evenienza alla missing deadline.
4. un evento di Hang, simile al delay ma nell’eventualità in cui il RTOS non vede concludere
mai i task, per essere entrato in un qualche loop infinito (o deadlock, o spinlock e similari).
5. un Crash: ovvero il sistema operativo smette di funzionare in modo repentino.

# Struttura OS
annotazioni su cosa far fare a main_blinky <br>
al momento: <br>

Numero task attuale --> 2 <br>
I task inseriscono e leggono dati da una Queue. <br>
Idee: <br>
* task che killa freeRTOS allo scadere di un timer : per evitare che il sistema vada in hang. Utile per monitorare errori 3 e 4.

* i task che producono outupt devono farlo su un file anzichè su stdout

* usare pià strutture di sistema possibile (mutex, timer, code, semafori ecc.)

#### Dubbi
come monitorare i ritardi (errore 3)?

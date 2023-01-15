# Implementacja wzorca pub/sub z wykorzystaniem socketów TCP

## Design

Aby otworzyć architecutre diagram należy skopiować zawartość pliku "Architecture diagram" i wkleić w pasek wyszukiwania przeglądarki.

## Opis wykorzystanego protokołu - TCP

TCP (Transmission Control Protocol) to protokół kontroli transmisji służący do przesyłania danych między procesami różnych maszyn. Należy do modelu TCP/IP. Jest to protokół połączeniowy, który korzysta z protokołu IP do wysyłania i odbierania danych. W odróżnieniu od UDP gwarantuje on transmisję z zachowaniem odpowiedniej kolejności dostarczanych pakietów oraz zapobiega powstawaniu duplikatów. TCP działa w trybie klient-serwer. Połączenie inicjuje klient, a zakończyć połączenie może każda ze stron.

Transmisja danych:

W celu weryfikacji wysyłki i odbioru używane są sumy kontrolne oraz sekwencyjne numery pakietów. Host będący odbiorcą potwierdza otrzymanie pakietów, o konkretnych numerach sekwencyjnych poprzez ustawienie flagi ACK. Jeśli, któreś z pakietów nie dotarły do odbiorcy to host będący nadawcą wysyła je ponownie.

Stany połączenia:

LISTEN – oznacza gotowość do nawiązania połączenia.

SYN-SENT – oznacza stan nawiązywania połączenia. Został już wysłany pakiet z flagą SYN i następuję oczekiwanie na pakiet SYN+ACK.

SYN-RECEIVED – oznacza, że otrzymano pakiet SYN oraz wysłano SYN+ACK. Trwa oczekiwanie na pakiet ACK.

ESTABLISHED – oznacza, że połączenie zostało prawidłowo nawiązane.

FIN-WAIT-1 – oznacza, że został wysłany pakiet FIN. Dane nadal mogą być odbierane, ale wysyłanie nie jest już możliwe.

FIN-WAIT-2 – oznacza, że otrzymano potwierdzenie własnego pakietu z flagą FIN oraz trwa oczekiwanie na pakiet FIN ze strony serwera.

CLOSE-WAIT – oznacza, że otrzymano już pakiet FIN oraz wysłano ACK. Trwa oczekiwanie na przesłanie własnego pakietu FIN.

CLOSING – oznacza, że trwa proces zamknięcia połączenia.

LAST-ACK – oznacza, że otrzymano i wysłano pakiet FIN oraz trwa oczekiwanie na ostatni już pakiet z flagą ACK.

TIME-WAIT – oznacza, że trwa oczekiwanie na to czy druga strona otrzymała potwierdzenie rozłączenia. W tym stanie połączenie nie może być dłużej niż 4 minuty.

CLOSED – oznacza, że połączenie zostało zamknięte.

## Uruchomienie serwera

Aby skompilować i uruchomić serwer należy użyć następującego polecenia:

```bash
gcc server_multi_thread.c && ./a.out
```

## Uruchomienie klienta

Aby uruchomić klienta należy zainstalować bibliotekę DearPyGui:

```bash
pip install dearpygui
```

następnie należy skompilować i uruchomić plik main.py

```bash
py main.py
```

## Komendy obsługiwane przez serwer

Program działa na zasadzie wysyłania przez klienta komend do serwera, które powodują wykonanie przez serwer odpowiednich akcji.

Obsługiwane komendy:
1 - pobierz listę topiców, stringi oddzielone średnikami<br/>
2;nazwa_topica - subskrybuj topic<br/>
3;wiadomość;nazwa_topica - wyślij wiadomość do danego topica<br/>
4;nazwa_topica - dodaj topic<br/>
5;nazwa_topica - usuń topic<br/>

Serwer został zaiplementowany metodą wielowątkową - każdy nowy klient, który podłączy się do serwera obsługiwany jest przez osobny wątek.<br/>
Gdy serwer odbierze komendę nr 2(nowy subskrybent) wtedy zapisuje uchwyt do klienta do tablicy struktur.<br/>
Następnie gdy client-publisher wyśle wiadomość do któregoś z topiców - wiadomość jest rozsyłana do wszystkich subskrybentów danego topica.<br/>

Obsługa komend na serwerze zaiplementowana jest za pomocą instrukcji switch-case. Każda komenda obsługiwana jest przez osobną funkcję.

## Obsługa przypadku gdy N(rozmiar wysyłanych danych) > wielkości bufora TCP

Obsługa tego przypadku zaimplementowana została za pomocą znaku końca danych - '#'.

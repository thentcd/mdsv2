# CWT Scalogram Analyzer

Program do analizy sygnałów wielokanałowych z wykorzystaniem ciągłej transformaty falkowej (CWT) z intuicyjnym interfejsem graficznym Qt6.

## Funkcjonalności

### Podstawowe możliwości:

- **Wczytywanie plików CSV** z sygnałami wielokanałowymi (format: czas;kanał1;kanał2;...)
- **Wybór kanału** do analizy z listy rozwijanej
- **Parametry sygnału**: liczba próbek, częstotliwość próbkowania
- **Selekcja fragmentu** sygnału do analizy za pomocą suwaków
- **Wybór falki** spośród trzech dostępnych: Morlet, Mexican Hat, Daubechies
- **Konfiguracja skal** transformaty (min, max, liczba kroków)

### Wizualizacja:

- **Oscylogram** - wykres sygnału w dziedzinie czasu z możliwością zoomowania
- **Skalogram** - reprezentacja 2D wyników CWT (skala vs czas) z mapą kolorów
- **Interaktywne wykresy** z informacjami o parametrach analizy

## Instalacja

### Automatyczna instalacja (Ubuntu/Debian):

```bash
# Sklonuj repozytorium
git clone <repository-url>
cd CWTScalogramAnalyzer

# Nadaj uprawnienia wykonywania
chmod +x build.sh

# Uruchom skrypt budowania
./build.sh
```

### Manualna instalacja:

```bash
# Instalacja zależności
sudo apt update
sudo apt install build-essential cmake qt6-base-dev qt6-base-dev-tools libfftw3-dev pkg-config

# Budowanie
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Uruchomienie
./bin/CWTScalogramAnalyzer
```

## Instrukcja użytkowania

### 1. Ładowanie sygnału

- Kliknij **"Load Signal File"** i wybierz plik CSV
- Program automatycznie rozpozna liczbę kanałów
- Wybierz kanał do analizy z listy rozwijanej

### 2. Konfiguracja parametrów sygnału

- **Samples**: liczba próbek (automatycznie wykrywana)
- **Sampling Rate**: częstotliwość próbkowania w Hz
- **Start/End Sample**: zakres analizy (suwaki)

### 3. Wybór parametrów falkowych

- **Wavelet Type**: wybór falki (Morlet, Mexican Hat, Daubechies)
- **Min/Max Scale**: zakres skal transformaty
- **Scale Steps**: liczba kroków skali (rozdzielczość)

### 4. Analiza CWT

- Kliknij **"Perform CWT Analysis"**
- Obserwuj postęp na pasku postępu
- Wyniki pojawią się w skalogramie

### 5. Interpretacja wyników

- **Oscylogram** (górny): sygnał w dziedzinie czasu
- **Skalogram** (dolny): intensywność dla różnych skal i czasów
- **Mapa kolorów**: niebieska (niska intensywność) → czerwona (wysoka)

## Format plików CSV

Program obsługuje pliki CSV z danymi wielokanałowymi:

```csv
czas;kanał1;kanał2;kanał3
0.000;-0.165;-0.325;0.150
0.008;-0.155;-0.315;0.140
0.016;-0.195;-0.305;0.160
...
```

**Uwagi:**

- Pierwszy słupek to zawsze czas
- Separatory: średnik (;) lub przecinek (,)
- Obsługiwane formaty liczbowe: dziesiętne z kropką
- Automatyczne wykrywanie częstotliwości próbkowania

# PUCCH FORMAT2 Block Code Modeling

Симулятор канала связи с блочным кодированием для 5G NR PUCCH FORMAT2.
Реализован на C++17, без сторонних зависимостей кроме [nlohmann/json](https://github.com/nlohmann/json) и [GTest](https://github.com/google/googletest).

## Сборка

Требования: CMake ≥ 3.16 (≤ 3.27), g++ ≤ 11.4 или clang ≤ 14, доступ к интернету (FetchContent скачает зависимости).

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

## Использование

```bash
./build/pucch_f2 <input.json>
```

Результат записывается в `result.json` в текущей директории.
При ошибке программа выводит описание в `stderr` и возвращает ненулевой код.

---

## Форматы JSON

### Режим `coding` — кодирование и модуляция

**Вход:**
```json
{
  "mode": "coding",
  "num_of_pucch_f2_bits": 4,
  "pucch_f2_bits": [1, 0, 1, 1]
}
```

**Выход (`result.json`):**
```json
{
  "mode": "coding",
  "qpsk_symbols": [
    "0.7071067812+0.7071067812j",
    "-0.7071067812+0.7071067812j",
    ...
  ]
}
```

---

### Режим `decoding` — демодуляция и декодирование

**Вход:**
```json
{
  "mode": "decoding",
  "num_of_pucch_f2_bits": 4,
  "qpsk_symbols": [
    "0.536+0.357j", "-0.612+0.489j", "0.701-0.523j",
    "0.435+0.612j", "-0.589+0.401j", "0.678-0.312j",
    "-0.445-0.534j", "0.612+0.478j", "-0.523+0.601j",
    "0.489-0.445j"
  ]
}
```

**Выход (`result.json`):**
```json
{
  "mode": "decoding",
  "num_of_pucch_f2_bits": 4,
  "pucch_f2_bits": [1, 0, 1, 1]
}
```

---

### Режим `channel simulation` — симуляция канала и расчёт BLER

**Вход:**
```json
{
  "mode": "channel simulation",
  "num_of_pucch_f2_bits": 11,
  "snr_db": 3.0,
  "iterations": 10000
}
```

**Выход (`result.json`):**
```json
{
  "mode": "channel simulation",
  "num_of_pucch_f2_bits": 11,
  "bler": 0.0423,
  "success": 9577,
  "failed": 423
}
```

Допустимые значения `num_of_pucch_f2_bits`: **2, 4, 6, 8, 11**.

---

## Тесты

```bash
cd build && ctest --output-on-failure
```

51 тест охватывает все компоненты: кодер, декодер, QPSK, канал, JSON I/O и полный пайплайн.

## Бенчмарк декодера

```bash
./build/pucch_f2_bench
```

Измеряет пропускную способность мягкого декодера (decode/sec) для каждого n.

## Кривые BLER

```bash
python3 scripts/plot_bler.py
```

Опции:
```
--binary PATH   путь к бинарнику (по умолчанию ./build/pucch_f2)
--out FILE      выходной файл графика (по умолчанию bler_curves.png)
--iters N       итераций на точку SNR (по умолчанию 5000)
```

Результат: `bler_curves.png` — кривые BLER(SNR) для кодов (20,2), (20,4), (20,6), (20,8), (20,11).

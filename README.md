# STM32 Bare-Metal Battery Management System (BMS) Firmware

This is a bare-metal C firmware project for an ARM Cortex-M (STM32F4 series) microcontroller. It is designed to run cell voltage acquisition, temperature reading, and safety interlock state machine control at register-level precision.

The codebase includes a Software-in-the-Loop (SIL) simulation harness allowing it to compile and run on macOS/Linux using standard gcc tooling, facilitating testing of the state machine and driver logic.

---

## 🛠️ Peripheral Register-Level Architecture

The project features direct register manipulation for STM32 hardware blocks:

1. **ADC + DMA Driver (`src/adc_dma.c`)**:
   - Configures `ADC1` control registers to run continuous analog conversions.
   - Configures `DMA2 Stream 0` in **Circular Mode** with **Memory Increment** and **16-bit Transfer Size** to continuously copy voltage conversions directly to a memory buffer without CPU overhead.
   - Uses the **DMA Transfer Complete Interrupt (`TCIE`)** to decode ADC counts to floating-point voltage values.

2. **I2C Temperature Sensor Driver (`src/i2c_sensor.c`)**:
   - Implements low-level register-based I2C transactions (generating Start bit, writing data, polling `SB`, `ADDR`, `TXE`, and `RXNE` status flags, reading with ACK/NACK, and issuing Stop bit).
   - Simulates communication with a TI TMP102 digital temperature sensor.

3. **Interrupt-Driven UART Ring Buffer (`src/uart_ringbuf.c`)**:
   - Configures `USART1` for transmission and receive interrupts (`RXNEIE`).
   - Implements a thread-safe, interrupt-safe **Circular Ring Buffer (Circular Queue)** in C for queuing incoming serial bytes.
   - Polling-based transmission uses status registers (`TXE` polling).

4. **Safety Controller ISR (`src/timer_isr.c`)**:
   - Configures general-purpose hardware timer `TIM2` to trigger Update Interrupts at **10Hz (100ms interval)**.
   - The Timer Interrupt Service Routine (`TIM2_IRQHandler`) runs the safety monitoring loop, assessing cell voltages (from DMA) and temperature values (via I2C).
   - Controls high-voltage safety relays via `GPIOA` Pin 5 output (written using the Bit Set/Reset Register `BSRR`).

---

## 📁 File Structure

- **`inc/stm32f4xx.h`**: Mappings and structs matching ARM CMSIS register representations.
- **`inc/bms.h`**: Global data structures, safety thresholds, and function declarations.
- **`src/adc_dma.c`**: Register configuration for ADC conversions and circular DMA streaming.
- **`src/i2c_sensor.c`**: Low-level I2C transaction controller and temp sensor reader.
- **`src/uart_ringbuf.c`**: USART driver with circular buffer queue.
- **`src/timer_isr.c`**: Hardware timer initialization, GPIO relay control, and 10Hz safety loop ISR.
- **`src/main.c`**: Entry point and simulation harness verifying fault injection and safety shutdown.

---

## ⚙️ Compilation & Running

### Compile the firmware
```bash
make
```

### Run the simulation
```bash
./bms_firmware_sim
```

### Clean build outputs
```bash
make clean
```

---

## 📊 Simulation Demo Output

When you run the compiled binary (`./bms_firmware_sim`), it executes the embedded state machine and outputs real-time peripheral states:

```text
==================================================================
     STM32 BARE-METAL BATTERY MANAGEMENT SYSTEM FIRMWARE          
==================================================================

--- Starting Hardware Event Loop (TIM2 & DMA Polling) ---

[t=0ms] | Relays GPIO-PA5: ON (HIGH) | State: NORMAL | Temp0: 24.5 C | Cell5: 3.73 V | Pack V: 44.3 V | Faults: 0x0
[t=100ms] | Relays GPIO-PA5: ON (HIGH) | State: NORMAL | Temp0: 24.5 C | Cell5: 3.73 V | Pack V: 44.3 V | Faults: 0x0
...
[t=700ms] | Relays GPIO-PA5: ON (HIGH) | State: NORMAL | Temp0: 24.5 C | Cell5: 3.73 V | Pack V: 44.3 V | Faults: 0x0

[SIMULATION EVENT] Injecting Thermal Overtemp Fault (Sensor 0 > 60 C)...
[t=800ms] | Relays GPIO-PA5: OFF (LOW) | State: FAULT | Temp0: 65.2 C | Cell5: 3.73 V | Pack V: 44.3 V | Faults: 0x1
[t=900ms] | Relays GPIO-PA5: OFF (LOW) | State: FAULT | Temp0: 65.2 C | Cell5: 3.73 V | Pack V: 44.3 V | Faults: 0x1
...
[t=1700ms] | Relays GPIO-PA5: OFF (LOW) | State: FAULT | Temp0: 65.2 C | Cell5: 3.73 V | Pack V: 44.3 V | Faults: 0x1

[SIMULATION EVENT] Clearing active faults. Resetting state to normal...
[t=1800ms] | Relays GPIO-PA5: ON (HIGH) | State: NORMAL | Temp0: 24.5 C | Cell5: 3.73 V | Pack V: 44.3 V | Faults: 0x0
[t=1900ms] | Relays GPIO-PA5: ON (HIGH) | State: NORMAL | Temp0: 24.5 C | Cell5: 3.73 V | Pack V: 44.3 V | Faults: 0x0
...
[t=2900ms] | Relays GPIO-PA5: ON (HIGH) | State: NORMAL | Temp0: 24.5 C | Cell5: 3.73 V | Pack V: 44.3 V | Faults: 0x0

==================================================================
     🟢 SIMULATION COMPLETE: ALL HARDWARE safety loops verified.  
==================================================================
```


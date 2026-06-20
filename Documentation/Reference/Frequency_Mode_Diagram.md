# Frequency Mode User Interactions

This document contains a Mermaid state diagram representing the user interactions in Frequency Mode for the HAMPOD device.

**Reference:** ICOMReader_Manual_v106.txt, Section 2 (Frequency Entry Mode)

---

## State Diagram

```mermaid
stateDiagram-v2
    direction TB
    
    [*] --> NormalMode
    
    state NormalMode {
        [*] --> Idle
        note right of Idle
            Default operating mode
            All standard keypad functions
        end note
    }
    
    NormalMode --> FrequencyMode: Press [#] (Enter Frequency Entry)
    
    state FrequencyMode {
        [*] --> VfoSelect
        
        state VfoSelect {
            [*] --> CurrentVfo
            CurrentVfo --> VfoA: Press [#]
            VfoA --> VfoB: Press [#]
            VfoB --> CurrentVfo: Press [#]
            note right of VfoSelect
                Cycles: Current → VFO A → VFO B → Current...
                Speaks VFO selection
            end note
        }
        
        VfoSelect --> DigitEntry: Press [0]-[9]
        
        state DigitEntry {
            [*] --> AccumulatingDigits
            
            AccumulatingDigits --> AccumulatingDigits: Press [0]-[9]\nAnnounces digit
            AccumulatingDigits --> DecimalEntered: Press [*]\nAnnounces "point"
            DecimalEntered --> AccumulatingDigits: Press [0]-[9]\nAnnounces digit
            
            note left of AccumulatingDigits
                Examples:
                • "7074" → 7.074 MHz
                • "14074" → 14.074 MHz  
                • "14.074" → 14.074 MHz
            end note
        }
        
        DigitEntry --> FrequencySubmitted: Press [#] (Enter)\nSends to radio
        
        state FrequencySubmitted {
            [*] --> ParseFrequency
            ParseFrequency --> CheckSpecial: Is special freq?
            
            CheckSpecial --> Special777: freq == 777
            CheckSpecial --> Special999: freq == 999
            CheckSpecial --> SetRadioFreq: Normal frequency
            
            Special777 --> AnnounceVersion: Announce product info\nand firmware version
            Special999 --> FactoryReset: Restore EEPROM defaults\nand reset device
            SetRadioFreq --> AnnounceNewFreq: Radio tunes to freq\nAnnounces frequency
        }
    }
    
    %% Exit paths from FrequencyMode
    FrequencyMode --> NormalMode: Press [*] twice\n(Second decimal clears)
    FrequencyMode --> NormalMode: Press [D]\n(Clear key - Cancelled)
    FrequencyMode --> NormalMode: Timeout expires\n(if Key Timeout enabled)
    FrequencySubmitted --> NormalMode: Frequency set successfully
    AnnounceVersion --> NormalMode: Version announced
    FactoryReset --> NormalMode: Device resets
```

---

## Detailed Key Mappings in Frequency Mode

| Key | Action | Behavior |
|-----|--------|----------|
| `[0]`-`[9]` | Digit entry | Appends digit to frequency buffer, announces digit |
| `[*]` | Decimal point | First press: inserts decimal, announces "point" |
| `[*]` (second) | Cancel | Second decimal clears entry, exits to Normal Mode |
| `[#]` | Enter/VFO cycle | Before digits: cycles VFO selection (A→B→Current) |
| `[#]` | Submit | After digits: sends frequency to radio |
| `[D]` | Clear | Clears entry, announces "Cancelled", exits to Normal Mode |

---

## Special Frequencies

| Input | Effect |
|-------|--------|
| `777` + `[#]` | Announces product information and firmware version |
| `999` + `[#]` | Restores EEPROM to factory defaults and resets device |

---

## Timeout Behavior

If the **Key Timeout** option is enabled in configuration (5-30 seconds):
- Partial frequency entry will be cleared after timeout
- HamPod returns to Normal Mode
- Announces timeout notification

---

## Auto-Decimal Feature

For convenience, the HAMPOD can auto-insert the decimal point for 4-5 digit entries:

| Input Digits | Resulting Frequency |
|--------------|---------------------|
| `7074` | 7.074 MHz |
| `14074` | 14.074 MHz |
| `21200` | 21.200 MHz |
| `28400` | 28.400 MHz |

**Note:** The original ICOMManual describes explicit decimal entry, but auto-decimal is a convenience feature in the HAMPOD implementation.

---

## Sequence Diagram: Typical Frequency Entry

```mermaid
sequenceDiagram
    participant User
    participant HamPod
    participant Radio
    
    User->>HamPod: Press [#]
    HamPod->>User: "Frequency Mode" / VFO announcement
    
    User->>HamPod: Press [#]
    HamPod->>User: "VFO A"
    
    User->>HamPod: Press [1]
    HamPod->>User: "One"
    
    User->>HamPod: Press [4]
    HamPod->>User: "Four"
    
    User->>HamPod: Press [*]
    HamPod->>User: "Point"
    
    User->>HamPod: Press [0]
    HamPod->>User: "Zero"
    
    User->>HamPod: Press [7]
    HamPod->>User: "Seven"
    
    User->>HamPod: Press [4]
    HamPod->>User: "Four"
    
    User->>HamPod: Press [#]
    HamPod->>Radio: Set frequency 14.074 MHz
    Radio->>HamPod: Acknowledge
    HamPod->>User: "Fourteen point zero seven four megahertz"
    
    Note over HamPod: Returns to Normal Mode
```

---

## Sequence Diagram: Cancel Frequency Entry

```mermaid
sequenceDiagram
    participant User
    participant HamPod
    
    User->>HamPod: Press [#]
    HamPod->>User: Enter Frequency Mode
    
    User->>HamPod: Press [1] [4]
    HamPod->>User: "One" "Four"
    
    User->>HamPod: Press [D]
    HamPod->>User: "Cancelled"
    
    Note over HamPod: Returns to Normal Mode
    Note over HamPod: Frequency unchanged
```

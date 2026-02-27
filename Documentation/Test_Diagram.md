```mermaid
stateDiagram-v2
    [*] --> ViewMode
    
    state ViewMode {
        [*] --> Idle
        Idle --> Searching: Key [/]
        Searching --> Idle: Key [ESC]
    }

    ViewMode --> EditMode: Key [E] or ?action=edit
    
    state EditMode {
        [*] --> Buffering
        Buffering --> Saving: Key [Cmd+S]
        Saving --> Buffering: Success
    }

    EditMode --> ViewMode: Key [ESC] or ?action=view
    
    note right of ViewMode
        URL Param: ?mode=read
    end note
    note right of EditMode
        URL Param: ?mode=write
    end note
    ```
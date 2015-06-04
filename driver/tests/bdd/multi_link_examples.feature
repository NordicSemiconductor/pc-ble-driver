Feature: multi_link_example

  Scenario: C examples test
    Given target 1 is running S130 as peripheral
    And target 2 is running S130 as central
    And native application 'driver/examples/advertising/gcc/main' is running towards target 1
    And native application 'driver/examples/multi_link/gcc/main' is running towards target 2
    When target 1 application prints 'Started advertising' within 10 second(s)
    And target 2 application prints '  * Example' within 5 second(s)
    And target 2 application prints 'Started scan with return code: 0x00' within 5 second(s)
    And target 1 application prints 'Connected, connection handle' within 10 second(s)
    And target 2 application prints 'Connected to device' within 10 second(s)
    And target 2 application prints '1 devices connected' within 10 second(s)
    And target 2 application prints 'Started scan with return code: 0x00' within 10 second(s)
    And target 2 application is terminated
    And target 2 is running blinky
    And target 1 application prints 'Disconnected' within 10 second(s)
    And target 1 application prints 'Started advertising' within 10 second(s)
    And target 1 application is terminated
    Then all the developers are happy :-)

  Scenario: Python examples test
    Given target 1 is running S130 as peripheral
    And target 2 is running S130 as central
    And Python application 'python/examples/advertising/main.py' is running towards target 1
    And Python application 'python/examples/multi_link/main.py' is running towards target 2
    When target 1 application prints 'Started advertising' within 10 second(s)
    And target 2 application prints '  * Example' within 5 second(s)
    And target 2 application prints 'Started scan with return code: 0x00' within 5 second(s)
    And target 1 application prints 'Connected, connection handle' within 10 second(s)
    And target 2 application prints 'Connected to device' within 10 second(s)
    And target 2 application prints '1 devices connected' within 10 second(s)
    And target 2 application prints 'Started scan with return code: 0x00' within 10 second(s)
    And target 2 application is terminated
    And target 2 is running blinky
    And target 1 application prints 'Disconnected' within 10 second(s)
    And target 1 application prints 'Started advertising' within 10 second(s)
    And target 1 application is terminated
    Then all the developers are happy :-)

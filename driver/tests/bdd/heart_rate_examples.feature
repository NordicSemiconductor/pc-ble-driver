Feature: heart_rate_example

  Scenario: C examples test
    Given target 1 is running S130 as peripheral
    And target 2 is running S130 as central
    And native application 'driver/examples/heart_rate_monitor/gcc/main' is running towards target 1
    And native application 'driver/examples/heart_rate_collector/gcc/main' is running towards target 2
    When target 1 application prints 'Started advertising' within 5 second(s)
    And target 2 application prints 'Scan started' within 5 second(s)
    And target 1 application prints 'Connected' within 10 second(s)
    And target 2 application prints 'Received service discovery' within 10 second(s)
    And user type '<newline>' in target 2 application
    And target 1 application prints 'Received event with ID: 1' within 10 second(s)
    And target 2 application prints 'Received handle value notification' within 5 second(s)
    And user type '<newline>' in target 2 application
    And target 2 application prints 'Setting HRM CCCD' within 5 second(s)
    And target 2 application prints 'Received write response' within 5 second(s)
    And target 2 application is terminated
    And target 2 is running blinky
    And target 1 application prints 'Disconnected' within 5 second(s)
    And target 1 application prints 'Started advertising' within 5 second(s)
    And target 1 application is terminated
    Then all the developers are happy :-)

  Scenario: Python examples test
    Given target 1 is running S130 as peripheral
    And target 2 is running S130 as central
    And Python application 'python/examples/heart_rate_monitor/main.py' is running towards target 1
    And Python application 'python/examples/heart_rate_collector/main.py' is running towards target 2
    When target 1 application prints 'Started advertising' within 5 second(s)
    And target 2 application prints 'Scan started' within 5 second(s)
    And target 1 application prints 'Connected' within 10 second(s)
    And target 2 application prints 'Received service discovery response' within 10 second(s)
    And user type '<newline>' in target 2 application
    And target 1 application prints 'Received event with ID: 1' within 10 second(s)
    And target 2 application prints 'Received handle value notification' within 5 second(s)
    And user type '<newline>' in target 2 application
    And target 2 application prints 'Setting HRM CCCD' within 5 second(s)
    And target 2 application prints 'Received write response' within 5 second(s)
    And target 2 application is terminated
    And target 2 is running blinky
    And target 1 application prints 'Disconnected' within 5 second(s)
    And target 1 application prints 'Started advertising' within 5 second(s)
    And target 1 application is terminated
    Then all the developers are happy :-)

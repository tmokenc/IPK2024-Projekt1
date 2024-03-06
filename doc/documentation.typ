#import "template.typ": project

#show: project.with(
  title: "IPK2024-chat",
  authors: (
    (name: "Nguyen Le Duy", 
    email: "xnguye27@stud.fit.vut.cz", 
    affiliation: "Brno University of Technology\nFaculty of Information Technology",
    postal: "", 
    phone: ""),
  ),
  logo: "vut.png",
  abstract: [
    This documentation outlines the implementation details of a chat client developed using the IPK2024 protocol, which leverages both TCP (Transmission Control Protocol) and UDP (User Datagram Protocol) for communication. The chat client provides users with the ability to engage in real-time text-based communication over a network, facilitating seamless interaction among multiple participants.
  ]
)

= Introduction
#lorem(200)

== Protocol
#lorem(200)

The most used protocols today is UDP and TCP, they both have advantages and disavantages.

#table(
  columns: (auto, auto),
  inset: 10pt,
  align: horizon,
  [*UDP*], [*TCP*],

  [Does not care],
  [Ensure that data is fully delivered],

  [Low network overhead],
  [High network overhead]

)

=== UDP
#lorem(200)

=== TCP
#lorem(200)

= Specification
#lorem(50)

= Implementation
#lorem(50)

== Client
Client, the heart of the program, where it handle main logic of the chat application.

#figure(
  image("main_loop_state_machine.png", width: 50%),
  caption: [
    Client's Main Loop State Machine
  ],
)
#lorem(200)

== Connection
The connection is responsible for openning, maintaining and closing a socket.
It contains _function pointers_ that can be set at the runtime of the program.
#lorem(200)

=== UDP
A variant of IPK2024 protocol that utilizing UDP protocol under the hood. Since the UDP does not provide any 
#lorem(200)

=== TCP
A variant of IPK2024 protocol that utilizing TCP protocol under the hood
#lorem(500)

== Bytes
#lorem(200)

== Payload
Define universal data type of a incoming/outcoming payload.

== Args
Commandline Argument parser turns the array of arguments into a structured data for the rest of the program to work with.

= Testing
Testing is a major part in development cycle. It validate the functionality of the of the project and make sure that they work as in the specification.

I use the `greatest` testing library#cite("greatest") to provide robust testing.

== Unit tests
#lorem(100)

== Connection tests
#lorem(200)

= Conclusion

#lorem(50)

#bibliography("refs.bib")

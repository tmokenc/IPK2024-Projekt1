#import "template.typ": project

#show: project.with(
  title: "IPK2024-chat",
  authors: (
    (name: "Nguyen Le Duy", 
    email: "xnguye27@stud.fit.vut.cz", 
    affiliation: "Brno University of Technology",
    postal: "", 
    phone: ""),
  ),
  abstract: lorem(50)
)

= Introduction
#lorem(200)

= Implementation
#lorem(200)

= Testing
Testing is a major part in development cycle. It validate the functionality of the of the project and make sure that they work as in the specification.

I use the `greatest` testing library#cite("greatest") to provide robust testing.
#lorem(50)

#bibliography("refs.bib")

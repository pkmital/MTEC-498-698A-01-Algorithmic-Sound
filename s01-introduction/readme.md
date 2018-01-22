# Algorithmic Sound

a.k.a. procedural sound, generative sound

* Algorithmic: Set of rules define the outcome.
* Nonlinear:  Building interaction into the narrative.
* Procedural:  Dependent on behavior.
* Agency:  Often the procedural nature indicates a sound object or some source.

Thinking about sound and soundscapes as a parametric, procedural, and generative process which we can describe with code.

# Why?

Media is becoming increasingly algorithmic.  Authoring tools still rely on heavy narrative elements.  Can we bring advances in pattern recognition, machine learning, and deep learning to creative sound practice?  If we can, then we can build smarter interaction, deeper algorithmic behavior, and more nonlinear and procedural sound into our compositions.

# Application Fields

* Film sound
* VR/AR
* Game design
* Musical sound practice
* Software development
* Installation artwork

# Reading Material

* _Audio/Vision_ by Michel Chion
* _Sonic Experience_ by Augoyard Torgue
* _Procedural Sound_ by Andy Farnell

# Examples

* RjDj
* Inception app
* No Man's Sky
* Spore
* Papa Sangre
* Pugs Luv Beats

# Synthesis Techniques

* AM
* FM
* Physical Models
* Additive
* Subtractive
* Concatenative
* Granular
* Binaural
* Ambisonic
* Wavefield

# Other tools

* PD, libPD
* Max/MSP
* Blueprint (Unreal Editor 4)
* SuperCollider
* Chuck
* Auraglyph
* Cinder

# Why C/C++?

* Extensible:  E.g., can write externals in C/C++ which are supported by most of the above tools.
* Fast:  Often audio demands high samplerate.  Working with procedural audio often demands many more resources than fixed media.  Multiply these together, and you need a very fast implementation capable of doing more.

# Why openFrameworks?

* Community:  A lot of support online and through other users.
* Libraries:  Integrates with nearly every C/C++ library out there.
* Creative:  Mostly focused on artists and creators.
* Quality:  Because it is so performant, you can often push more boundaries than in any other platform.

# Downsides?

* Low-level:  Often hard to iterate on ideas working at the level of C/C++, as compared to node-based languages.
* Portability:  Often the SDKs which your application are built against only last 1-2 years.
* Difficult:  Related to the other downsides, but it is often a lot more difficult because it is so much more of a blank canvas.

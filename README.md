# TripleAgent

## Overview
Non-COM open-source reimplementation of Microsoft Agent Technology

## What this project is
This project offers only the API to create and display the animated characters, speech balloons and to synthesize speech. It doesn't aim to implement the speech based command system of the original API.

## The current implementation

  |Module|Status|Description|
  |------|------|-----------|
  |Core|In progress|Responsible for request handling, animation system and balloon interactions|
  |Windowing|Windows only|Manages the Agent and Balloon windows. Currently relies on Windows APIs, but it isn't platform-specific by design.|
  |DataProvider|Completed|Responsible for parsing the Agent Character Specification (.ACS) files.|
  |UserAgent|-|Test project for validating library components.|

## Planned features
- Modular balloon rendering engine
- Cross-platform runtime (Windows, Linux, macOS, Android)
- Text-to-speech integration

# MSAgent quirks
Read first: [MSDN](https://learn.microsoft.com/en-us/windows/win32/lwef/microsoft-agent)

  ## Animation system
  ### Null frame
  The animation system doesn't display frames that have the duration of zero (called here `null frames`),<sup>[\[ref\]](https://learn.microsoft.com/en-us/windows/win32/lwef/creating-animations)</sup> but it does respect the frame's branching and exit frame. 

  ### "Speaking frame" of an animation
  MSAgent's animation system requires the speaking frame to be last. To work around this, some characters make the speaking frame jump to a null frame, ending the animation so the final valid frame is the one preceding the jump.
  
  ### Moving an Agent
  The specific details are unknown at the moment. I'm working under the assumption that it works more or less like the `Speaking frame`. The agent plays the right move animation, then it skips to the null frame, moves to the desired position and finishes the animation.

## Working theory
The first time I implemented the core loop I had closely tied the animation system and request system. Looking back I think that is not solution to the problem. All these complex tricks and techniques derived from the technology hint to some sort of frame-level FSM.

# Disclaimer
I am aware of the existence of Double Agent (hence the name *Triple*Agent) but none of its code is present on this codebase. This project is kind of a pet project of mine and I didn't want to just copy someone else's implementation.

# Regards
- Thanks to Remy Lebeau [Lebeau Software](http://lebeausoftware.org/) for the file format specification.
- Thanks to Double Agent [Cinnamon Software](https://doubleagent.sourceforge.net/) for keeping this technology around.

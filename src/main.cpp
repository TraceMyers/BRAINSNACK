// ---------------------------------------------------------------------------------------------------------------------
// A NOTE FOR RECRUITERS AND OTHER COOL PEOPLE
// ---------------------------------------------------------------------------------------------------------------------
//
// hey there! thanks for checking out the source code for BRAINSNACK.
// 
// i wrote the source for this game, including the c++, html, css and *some* of the javascript
// from roughly Jan 1, 2026 to ?, working something close to full time hours (but we're also moving, so there's that). 
// a lot of the work was translated into c++ from older jai projects, so, you know, how much time it took to do is 
// kind of subjective.
//
// i'm not super interested in having an ai write code for me, though i will sometimes have it generate snippets
// and i will copy that code. i also use it very often for sanity checks and learning new things that i think are 
// well-known. i expect the ai will understand those topics well.
//
// my goal with this project was to show a cross-section of a game that was built 'from scratch' (as it goes)
// with an experienced perspective on which code needs to be more more solid + generic (these are different!), and 
// which of it needs to be done the quick-and-easy way.
//
// it's important to understand when things need to be more complex in order to support functionality on higher layers,
// because if you build on top of very prototypey code, you end up with an insufficient interface that engenders 
// boilerplate and spiderweb logic. and, those things *also* represent complexity. it's maybe less percievable, but it 
// does just as much to make the project harder to work on. 
//
// you probably expect to see code that looks like what i would write in your codebase.
// unfortunately, i don't think this is a realistic expectation, because it's impossible to make a codebase that trying 
// to be exactly as complex as it needs to be *and* also make it representative of likewise code written in the midst of 
// a far more complex codebase.
//
// for example, i like writing multithreaded systems, but intentional multithreading would be needlessly complex
// for this game.
//
// so, maybe a little guidance is due.
//
// if you want to see code that represents foundational software engineering, take a look at the allocators/
// folder as well as strings (string.h/string.cpp) and printing (print.h/print.cpp). the array interface and the 
// pool interface are somewhat incomplete for a larger project, but you can tell what they would be, were they made to 
// be more complex, with more functionality.
//
// if you want to see 'quick an easy', you might be spoiled for choice, but maybe check out functions like RenderFrame()
// in graphics/renderer.cpp and the mesh loading in graphics/mesh.cpp.
//
// there is a bit of a mix of imperative and cpp-style object-oriented, but in general I lean away from object-oriented
// things like virtual functions, inheritance and interfaces. As a part of my complexity management, I strongly
// believe that avoiding deep inheritence saves the programmer from a lot of headaches. single-layer
// inheritance is useful, but i prefer composition and switch statements.
//
// i can use libraries fine, but i avoided using them wherever practical here, primarily to demonstrate that i could
// (and hopefully will be) hired to write your foundational systems. 
//
// comments are sometimes a contentious subject. in general, i'd say that the number of comments should go up as the 
// number of brand new readers increases, and down to a base level of 'only the complicated stuff' as the number of 
// people regularly working in that part of the codebase increases.
//
// this project was written in an unreal-ish style because that's what i'm used to when writing c++. i don't
// have strong preferences in code style. if you're curious, here are the style rules:
//
// - everything is capitalized, without underscores
// - boolean names start with lowercase 'b'
// - structs without (or with a very small number of) methods are prefixed with 'F' 
// - structs that are more method-heavy use the 'class' keyword and are prefixed with 'T' (does not indicate templating)
// - braces go on new lines
//
// if you come across this code before seeing the game, hopefully it's still available to play at my portfolio site,
// tracemyers.com.
//
// feel free to send an email to me at main@tracemyers.com
//
// - Trace 
//
// ---------------------------------------------------------------------------------------------------------------------
// WHERE TO GET STARTED READING
// ---------------------------------------------------------------------------------------------------------------------
//
// emscripten glues the webassembly-compiled code into a javascript environment for running in browser.
// but, if you want the native program analog, it works something like this:
//
//  int main()
//  { 
//      InitGame();             <--------- this way for iniitialization
//      while (!bQuit) 
//      { 
//          TickGame();         <--------- this way for everything else
//          yield ToBrowser();
//      } 
//      return 0;
//  }
//
// ---------------------------------------------------------------------------------------------------------------------

#include <stdio.h>

#include <emscripten.h>
#include <emscripten/html5.h>

#include "core.h"
#include "tests.h"

// ---------------------------------------------------------------------------------------------------------------------

void TickGame(void*);
void InitGame();

// ---------------------------------------------------------------------------------------------------------------------

int main()
{
    InitGame();
    return 0;
}

void InitGame()
{
    Input.Init();
    Session.Init();
    Renderer.Init();
    // give emscripten some callbacks to be called from javascript (or html5?) code on the webpage.
    // note I think that doing javascript codegen without html codegen makes it so the html5-dependent bindings don't work, which is fine. don't need.
    if (!Session.Quitting())
    {
        // emscripten_set_main_loop(TickGame, 0, true);
        emscripten_set_main_loop_arg(TickGame, nullptr, 0, 1);
    }
}

// webassembled version of this is called per-frame by emscripten-generated javascript
void TickGame(void* UserData)
{
    Session.BeginFrame();
    DEFER_CALL(Session, EndFrame)

    const float DeltaTime = Session.DeltaTime();

    Input.FrameUpdate(DeltaTime);
    ObjectMovement.FrameUpdate(DeltaTime);
    Renderer.RenderFrame(DeltaTime);
}
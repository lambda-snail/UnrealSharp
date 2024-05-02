# TL;DR

UnrealSharp attempts to add a `dotnet core` integration to some parts of Unreal Engine. It does this by creating what the `dotnet` documentation calls a "native host" in the context of a subsystem, and exposes functionality by providing function pointers to `dotnet`. You can read more about this in my other repository [dotnet-host-example](https://github.com/lambda-snail/dotnet-host-example) where I play around with an example from Microsoft, and document some cases that were not really covered in the official channels (as far as I could tell).

The plugin is currently in an early stage and is probably not very useful at the moment.

# Current Scope

The scope for version 1.0 is actor (and maybe character) integration and reducing boilerplate. In the future other classes may be supported as well.

My dream is to have a plugin where the user (developer) can work with his/her own `dotnet` code, using any `nuget` package they want etc., and using this without touching any of the internals of the plugin (this is currently impossible).

# Current State

## Ticking

Currently it is possible to register an actor with the subsystem, and have it tick each frame using code written in `csharp`. The `Actor` class in `dotnet` has properties for individually setting and getting the translation, rotation and scale of the C++ `Actor`, as well as getting and setting the `Transform`. The example provided uses `dotnet` to make an `Actor` float up and down:

```csharp
public class TestActor : Actor
{
    private bool _shouldGoUp = true;
    private readonly float _speed = 100f;
    
    public override void Tick(float deltaTime)
    {
        Vector vector = Translation;

        float direction = _shouldGoUp ? 1f : -1f;
        vector.Z += (_speed * deltaTime * direction);
        if (vector.Z >= 400f || vector.Z <= 50f)
        {
            _shouldGoUp = !_shouldGoUp;
        }
        
        Translation = vector;
    }
}
```

This is achieved by creating `extern "C"` bindings which can then be used to P/Invoke back into the executable. We keep track of which C++ `Actor` instance is bound to which C# instance by storing a pointer to the actor, which is in turn used to get and set things on the native side. This requires some ugly things but these can be hidden in properties in the base class in C":

```csharp
public Vector Translation
{
    get
    {
        ActorManager.GetTranslation(ActorPtr, out Vector vector);
        return vector;
    }
    set
    {
        ActorManager.SetTranslation(ActorPtr, ref value);
    }
}
```

As can be seen in the example, this allows us to expose a fairly intuitive api to the C# user:

```csharp
Vector vector = Translation;
...
Translation = vector;
```

## Generating Bindings

I am currently working on a Unreal Header Tool plugin that will generate the bindings when building the C++ code. This is done by specifying additional information in the `meta` tag of the `UPROPERTY` macro like so:

```c++
UPROPERTY(meta = (DotnetReadWrite = "true")) 
uint64 Int64Prop; 
``` 

This will generate binding code assuming that the property is public:

```c++
extern "C" __declspec(dllexport) inline void Get_Int64Prop(AUnrealSharpDemoActor const* Instance, void* Parameter) {
    auto* TypedPtr = static_cast<uint64*>(Parameter);
    *TypedPtr = Instance->Int64Prop;
}
```

Maybe we don't want to expose our properties for everyone to read and modify, in which case we can either use the reflection system:

```c++
UPROPERTY(meta = (DotnetReadWrite = "true", DotnetAccess = "Reflection")) 
uint32 Int32Prop;
```

Which leads to the following bindings getting generated:

```c++
extern "C" __declspec(dllexport) inline void Get_Int32Prop(AUnrealSharpDemoActor const* Instance, void* Parameter) {
    auto* TypedPtr = static_cast<uint32*>(Parameter);
    static FProperty* Property = AUnrealSharpDemoActor::StaticClass()->FindPropertyByName("Int32Prop");
    uint32 Val;Property->GetValue_InContainer(Instance, &Val);
    *TypedPtr = Val;
}
```

We can also provide our own member function for getting and setting:

```c++
private:
UPROPERTY(meta = (DotnetReadWrite = "true", DotnetAccess = "DoubleProp"))
double  DoubleProp;

public:
double GetDoubleProp() const { return {}; };
void SetDoubleProp(double Prop) {};
```

"DoubleProp" forms the base for the member function name and will generate bindings for `GetDoubleProp` and `SetDoubleProp`:

```c++
extern "C" __declspec(dllexport) inline void Get_DoubleProp(AUnrealSharpDemoActor const* Instance, void* Parameter) {
auto* TypedPtr = static_cast<double*>(Parameter);
*TypedPtr = Instance->GetDoubleProp();
}
```

At the current stage, these haven't been tested yet, so may work like a charm or not at all. I will update this README as I progress.

# In Progress

- [ ] Extend UHT: Design a way to automate the boilerplate required to set up a read/write of some data.

# Todo (actor integration)

More functionality will be coming when I have the time :)

- [ ] Clean up the `Host` files and incorporate into the subsystem where approproate. "Unrealify" the coding style.
- [ ] See if we can build the `UnrealSharpCore` assembly automatically as part of Unreal's normal buid process.
- [ ] Set up communication between `dotnet` and `cpp` to allow developers to read and write common data (e.g., transform data etc).
- [ ] Decide how to handle common math operations etc that will be needed on the `dotnet` side. E.g., do we make `FVector` member functions available through callbacks, do we provide some external package for math operations, or do we leave the users to their own fates in this regard?
    - Thought: Since we are on dotnet core, the user could in theory use any `nuget` package they wish.
- [ ] Decide what data to support (e.g., do we allow `dotnet` to get pointers to somehow subsystems, do we make it possible to manipulate animations etc.)
- [ ] Set up a way for developers to provide their own `dotnet` assembly with their own types and register that in the system.
- [ ] `Nice to have` Design a clean way for developers to extend the integration, by adding their own read and write callbacks for custom data.
- [ ] Some example classes and documentation

# License and Attribution

This project is licensed under the MIT license. It contains modified code from the official dotnet samples repository found here: https://github.com/dotnet/samples/tree/main/core/hosting

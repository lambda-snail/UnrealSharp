# TLDR

UnrealSharp attempts to add a `dotnet core` integration to some parts of Unreal Engine. It does this by creating what the `dotnet` documentation calls a "native host" in the context of a subsystem, and exposes functionality by providing function pointers to `dotnet`. You can read more about this in my other repository [dotnet-host-example](https://github.com/lambda-snail/dotnet-host-example) where I play around with an example from Microsoft, and document some cases that were not really covered in the official channels (as far as I could tell).

The plugin is currently in an early stage and is probably not very useful at the moment.

# Current Scope

The scope for version 1.0 is actor (and maybe character) integration and reducing boilerplate. In the future other classes may be supported as well.

My dream is to have a plugin where the user (developer) can work with his/her own `dotnet` code, using any `nuget` package they want etc., and using this without touching any of the internals of the plugin (this is currently impossible).

# Current State

Currently it is possible to register an actor with the subsystem, and have it tick each frame using code written in `csharp`. The location of the actor can be read from and written to, but not much else.

```csharp
public class TestActor : Actor
{
    private bool _shouldGoUp = true;
    private readonly float _speed = 100f;
    
    public override void Tick(float deltaTime)
    {
        Transform t = GetTransform();

        float direction = _shouldGoUp ? 1f : -1f;
        t.Location.Z += (_speed * deltaTime * direction);
        if (t.Location.Z >= 400f || t.Location.Z <= 50f)
        {
            _shouldGoUp = !_shouldGoUp;
        }
        
        UELog.Log($"Actor with handle {ActorHandle}: Transform=({t.Location.X},{t.Location.Y},{t.Location.Z})");
        SetTransform(t);
    }
}
```

More functionality will be coming when I have the time :)

# Todo (actor integration)

- [ ] Clean up the `Host` files and incorporate into the subsystem where approproate. "Unrealify" the coding style.
- [ ] Set up communication between `dotnet` and `cpp` to allow developers to read and write common data (e.g., transform data etc).
- [ ] Design a way to automate the boilerplate required to set up a read/write of some data.
    - Possible solution: Extend the UHT
    - Python scripts?
- [ ] Decide how to handle common math operations etc that will be needed on the `dotnet` side. E.g., do we make `FVector` member functions available through callbacks, do we provide some external package for math operations, or do we leave the users to their own fates in this regard?
    - Thought: Since we are on dotnet core, the user could in theory use any `nuget` package they wish.
- [ ] Decide what data to support (e.g., do we allow `dotnet` to get pointers to somehow subsystems, do we make it possible to manipulate animations etc.)
- [ ] Set up a way for developers to provide their own `dotnet` assembly with their own types and register that in the system.
- [ ] `Nice to have` Design a clean way for developers to extend the integration, by adding their own read and write callbacks for custom data.
- [ ] Some example classes and documentation

# License and Attribution

This project is licensed under the MIT license. It contains modified code from the official dotnet samples repository found here: https://github.com/dotnet/samples/tree/main/core/hosting

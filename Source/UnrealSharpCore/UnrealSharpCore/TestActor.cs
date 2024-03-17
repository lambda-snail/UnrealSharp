using System.Security.Cryptography;

namespace LambdaSnail.UnrealSharp;

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
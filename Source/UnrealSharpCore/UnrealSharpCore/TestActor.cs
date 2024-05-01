using System.Security.Cryptography;

namespace LambdaSnail.UnrealSharp;

public class TestActor : Actor
{
    private bool _shouldGoUp = true;
    private readonly float _speed = 100f;
    
    public override void Tick(float deltaTime)
    {
        Transform t = ActorManager.GetTransform(ActorPtr);

        float direction = _shouldGoUp ? 1f : -1f;
        t.Translation.Z += (_speed * deltaTime * direction);
        if (t.Translation.Z >= 400f || t.Translation.Z <= 50f)
        {
            _shouldGoUp = !_shouldGoUp;
        }
        
        //UELog.Log($"Actor with handle {ActorHandle}: Transform=({t.Translation.X};{t.Translation.Y};{t.Translation.Z})");
        ActorManager.SetTransform(ActorPtr, t);
    }
}
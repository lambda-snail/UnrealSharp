using System.Runtime.InteropServices;
using System.Security.Cryptography;

namespace LambdaSnail.UnrealSharp;

public class TestActor : Actor
{
    private bool _shouldGoUp = true;
    private readonly float _speed = 100f;

    public override void Tick(float deltaTime)
    {
        //ActorManager.GetTranslation(ActorPtr, out Vector vector);
        Vector vector = Translation;

        float direction = _shouldGoUp ? 1f : -1f;
        vector.Z += (_speed * deltaTime * direction);
        if (vector.Z >= 400f || vector.Z <= 50f)
        {
            _shouldGoUp = !_shouldGoUp;
        }
        
        //ActorManager.SetTranslation(ActorPtr, ref vector);
        Translation = vector;
    }
}
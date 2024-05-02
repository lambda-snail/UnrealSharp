using System.Runtime.InteropServices;
using System.Security.Cryptography;

namespace LambdaSnail.UnrealSharp;

public class TestActor : Actor
{
    private bool _shouldGoUp = true;
    private readonly float _speed = 100f;

    public override void Tick(float deltaTime)
    {
        Transform t = ActorManager.GetTransform(ActorPtr);

        try
        {
            // Vector vector = new Vector();
            // ActorManager.GetTranslation(ActorPtr, ref vector);
            // UELog.Log($"Translation is: ({vector.X};{vector.Y};{vector.Z})");
            
            Vector vector = ActorManager.GetTranslation(ActorPtr);
            UELog.Log($"Translation is: ({vector.X};{vector.Y};{vector.Z})");
        }
        catch (Exception e)
        {
            UELog.Log(e.Message);
            UELog.Log(e.StackTrace!);
        }

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
Imports LibKNDotNet

Public MustInherit Class xTU_SubListener

    Inherits LibKNDotNet.IListener

    Private Sub Listener_OnUpdate(ByVal msg As LibKNDotNet.Message)
        TU_dumpMsg("TU_SubListener_OnUpdate", msg)
        tu_count_subListener_OnUpdate = tu_count_subListener_OnUpdate + 1
    End Sub

End Class




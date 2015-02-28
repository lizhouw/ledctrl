package android.flukenetworks;

import android.content.Context;
import android.os.ServiceManager;
import android.util.Log;
import android.os.RemoteException;
import com.android.internal.flukenetworks.ILedctrlService;

public class FlukeNetworksService {

    private static final String LOG_TAG = "FlukeNetworks_Service";
    private static ILedctrlService  ledctrl_service;

    public FlukeNetworksService(Context context){
        if (ledctrl_service == null) {
            ledctrl_service = ILedctrlService.Stub.asInterface(ServiceManager.getService(                                                          
                                                               "LedctrlService"));
        }
    }

    public void ledOn(){
        try{
            ledctrl_service.setStatus(1);
        } 
        catch (RemoteException ex) {
        }
    }

    public void ledOff(){
        try{
            ledctrl_service.setStatus(0);
        }
        catch (RemoteException ex) {
        }
    }

    public int ledStatus(){
        try{
            return ledctrl_service.getStatus();
        }
        catch (RemoteException ex){
            return -1;
        }
    }
}


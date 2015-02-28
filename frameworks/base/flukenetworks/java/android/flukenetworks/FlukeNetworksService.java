package android.flukenetworks;

import android.content.Context;
import android.os.ServiceManager;
import android.util.Log;
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
        ledctrl_service.setStatus(1);
    }

    public void ledOff(){
        ledctrl_service.setStatus(0);
    }

    public int ledStatus(){
        return ledctrl_service.getStatus();
    }
}


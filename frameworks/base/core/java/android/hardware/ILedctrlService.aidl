package android.hardware;

/**
 * {@hide}
 */

interface ILedctrlService {
    void setStatus (int val);
    int  getStatus ();
}

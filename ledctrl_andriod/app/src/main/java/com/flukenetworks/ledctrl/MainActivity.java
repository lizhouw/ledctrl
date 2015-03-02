package com.flukenetworks.ledctrl;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.Button;
import android.widget.EditText;
import android.flukenetworks.FlukeNetworksService;
import android.util.Log;

public class MainActivity extends Activity implements OnClickListener{

    private final static String LOG_TAG = "com.flukenetworks.ledctrl";

    private FlukeNetworksService fluke_services;
    private EditText text_value = null;
    private Button button_getstatus = null;
    private Button button_ledon = null;
    private Button button_ledoff = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        fluke_services = (FlukeNetworksService)getSystemService("flukenetworks");

        text_value = (EditText)findViewById(R.id.edit_ledstatus);
        button_getstatus = (Button)findViewById(R.id.button_getstatus);
        button_ledon = (Button)findViewById(R.id.button_ledon);
        button_ledoff = (Button)findViewById(R.id.button_ledoff);

        button_getstatus.setOnClickListener(this);
        button_ledon.setOnClickListener(this);
        button_ledoff.setOnClickListener(this);

        Log.i(LOG_TAG, "LedCtrl activity created");
    }

    @Override
    public void onClick(View v) {
        if(v.equals(button_getstatus)) {
            int led_status;
            led_status = fluke_services.ledStatus();
            text_value.setText((0 == led_status) ? "OFF" : "ON");
        }
        else {
            if (v.equals(button_ledon)) {
                fluke_services.ledOn();
            }
            else {
                fluke_services.ledOff();
            }
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }
}

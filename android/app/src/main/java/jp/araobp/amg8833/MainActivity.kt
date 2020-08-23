package jp.araobp.amg8833

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import jp.araobp.amg8833.serial.Amg8833Interface
import jp.araobp.amg8833.visualizer.R

class MainActivity : AppCompatActivity() {

    private var baudrate: Int = 9600
    private lateinit var amg8833Interface: Amg8833Interface

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        
    }
}
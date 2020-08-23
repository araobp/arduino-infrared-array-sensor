package jp.araobp.amg8833.analyzer

import android.graphics.Color
import android.graphics.Paint

fun paintGrayscale(brightness: Int) = Paint().apply {
    style = Paint.Style.FILL_AND_STROKE
    color = Color.rgb(brightness, brightness, brightness)
}


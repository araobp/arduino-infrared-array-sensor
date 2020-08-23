package jp.araobp.amg8833.analyzer

import android.view.SurfaceView

class RawImage(val surfaceView: SurfaceView) {

    fun draw(data: UByteArray) {

        val normalizedData = minMaxNormalize(data)

        val canvas = surfaceView.holder.lockCanvas()

        val width = canvas.width.toFloat()
        //val height = canvas.height.toFloat()

        val xStep = (width * 9F / 10F) / 8F
        val yStep = xStep

        for (row in 0 until 8) {
            for (col in 0 until 8) {
                val pixel = normalizedData[row * 8 + col]
                val left = xStep * col
                val top = yStep * row
                val right = xStep * (col + 1)
                val bottom = yStep * (row + 1)
                val paint = paintGrayscale(pixel.toInt())
                canvas.drawRect(left, top, right, bottom, paint)
            }
        }
        surfaceView.holder.unlockCanvasAndPost(canvas)
    }
}
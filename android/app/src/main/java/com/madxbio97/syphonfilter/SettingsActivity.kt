package com.madxbio97.syphonfilter

import android.os.Bundle
import android.widget.ArrayAdapter
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import com.madxbio97.syphonfilter.databinding.ActivitySettingsBinding
import java.io.File

class SettingsActivity : AppCompatActivity() {

    private lateinit var binding: ActivitySettingsBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivitySettingsBinding.inflate(layoutInflater)
        setContentView(binding.root)

        val fpsOptions = listOf(
            getString(R.string.fps_60),
            getString(R.string.fps_30),
            getString(R.string.fps_120),
            getString(R.string.fps_uncapped)
        )
        val adapter = ArrayAdapter(this, android.R.layout.simple_spinner_dropdown_item, fpsOptions)
        binding.spFps.adapter = adapter

        loadSettings()

        binding.btnSave.setOnClickListener {
            saveSettings()
            Toast.makeText(this, R.string.save_settings, Toast.LENGTH_SHORT).show()
            finish()
        }
    }

    private fun getConfigFile(): File {
        val dir = File(filesDir, "SyphonFilterPC")
        if (!dir.exists()) dir.mkdirs()
        return File(dir, "launcher.ini")
    }

    private fun loadSettings() {
        val file = getConfigFile()
        if (!file.exists()) return

        val lines = file.readLines()
        for (line in lines) {
            val parts = line.split("=")
            if (parts.size != 2) continue
            val key = parts[0].trim()
            val value = parts[1].trim()

            when (key) {
                "Aspect" -> {
                    if (value == "0") binding.rbAspectAdaptive.isChecked = true
                    else binding.rbAspect43.isChecked = true
                }
                "VSync" -> binding.swVsync.isChecked = (value == "1")
                "VolumetricEffects" -> binding.swVolumetrics.isChecked = (value == "1")
                "MissionSkyboxes" -> binding.swSkyboxes.isChecked = (value == "1")
                "TouchOverlay" -> binding.swTouchOverlay.isChecked = (value != "0")
                "TouchHaptics" -> binding.swTouchHaptics.isChecked = (value != "0")
                "Vibration" -> binding.swVibration.isChecked = (value != "0")
                "CheatAllWeapons" -> binding.swCheatWeapons.isChecked = (value == "1")
                "CheatOneShot" -> binding.swCheatOneshot.isChecked = (value == "1")
                "CheatWeak" -> binding.swCheatWeak.isChecked = (value == "1")
                "CheatMovie" -> binding.swCheatMovie.isChecked = (value == "1")
                "FrameLimit" -> {
                    val fps = value.toIntOrNull() ?: 60
                    when (fps) {
                        30 -> binding.spFps.setSelection(1)
                        120 -> binding.spFps.setSelection(2)
                        0 -> binding.spFps.setSelection(3)
                        else -> binding.spFps.setSelection(0)
                    }
                }
            }
        }
    }

    private fun saveSettings() {
        val file = getConfigFile()
        val aspect = if (binding.rbAspectAdaptive.isChecked) "0" else "1"
        val vsync = if (binding.swVsync.isChecked) "1" else "0"
        val volumetrics = if (binding.swVolumetrics.isChecked) "1" else "0"
        val skyboxes = if (binding.swSkyboxes.isChecked) "1" else "0"
        val touchOverlay = if (binding.swTouchOverlay.isChecked) "1" else "0"
        val touchHaptics = if (binding.swTouchHaptics.isChecked) "1" else "0"
        val vibration = if (binding.swVibration.isChecked) "1" else "0"
        val cheatWeapons = if (binding.swCheatWeapons.isChecked) "1" else "0"
        val cheatOneshot = if (binding.swCheatOneshot.isChecked) "1" else "0"
        val cheatWeak = if (binding.swCheatWeak.isChecked) "1" else "0"
        val cheatMovie = if (binding.swCheatMovie.isChecked) "1" else "0"

        val frameLimit = when (binding.spFps.selectedItemPosition) {
            1 -> "30"
            2 -> "120"
            3 -> "0"
            else -> "60"
        }

        val iniContent = buildString {
            appendLine("[Graphics]")
            appendLine("Aspect=")
            appendLine("VSync=")
            appendLine("VolumetricEffects=")
            appendLine("MissionSkyboxes=")
            appendLine("FrameLimit=")
            appendLine("TouchOverlay=")
            appendLine("TouchHaptics=")
            appendLine()
            appendLine("[Controller]")
            appendLine("Vibration=")
            appendLine()
            appendLine("[Cheats]")
            appendLine("AllWeapons=")
            appendLine("OneShotKills=")
            appendLine("WeakEnemies=")
            appendLine("MovieTheater=")
        }

        file.writeText(iniContent)
    }
}

/*
 * OptiPNG file type
 * Copyright (C) 2008 ilikepi3142@gmail.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

using PaintDotNet;
using System;

namespace ILikePi.FileTypes.OptiPng
{
    internal enum ColorMode
    {
        Grayscale,
        RGB,
        Palette,
        GrayscaleAlpha,
        RGBAlpha
    }

    [Serializable]
    internal class OptiPngSaveConfigToken : SaveConfigToken
    {
        private byte compression;
        private byte ditheringLevel;
        private byte threads;

        public OptiPngSaveConfigToken()
        {
            Color = ColorMode.RGBAlpha;
            Compression = 2;
            ThreadCount = 0;
            DitheringLevel = 7;
            TransparencyThreshold = 128;
            MultiplyByAlphaChannel = true;
            Interlace = false;
            Optimize = true;
            Quiet = true;
            LibDeflater = false;
            Zopfli = false;
        }

        public override object Clone()
        {
            return MemberwiseClone();
        }

        public override void Validate()
        {
            validateCompression(compression);
            validateDitheringLevel(ditheringLevel);
        }

        public override bool Equals(object obj)
        {
            if (obj is OptiPngSaveConfigToken token)
            {
                return token.Color == Color
                    && token.compression == compression
                    && token.ditheringLevel == ditheringLevel
                    && token.TransparencyThreshold == TransparencyThreshold
                    && token.Interlace == Interlace
                    && token.MultiplyByAlphaChannel == MultiplyByAlphaChannel
                    && token.Optimize == Optimize
                    && token.ThreadCount == threads
                    && token.Quiet == Quiet
                    && token.Zopfli == Zopfli
                    && token.LibDeflater == LibDeflater;
            }

            return false;
        }

        public override int GetHashCode()
        {
            return (int)Color + compression * 3 + ditheringLevel * 7;
        }

        public ColorMode Color { get; set; }

        public byte Compression
        {
            get
            {
                return compression;
            }
            set
            {
                validateCompression(value);
                compression = value;
            }
        }

        public byte ThreadCount
        {
            get
            {
                return threads;
            }

            set
            {
                validateThreads(value);
                threads = value;

            }
        }

        public byte DitheringLevel
        {
            get
            {
                return ditheringLevel;
            }
            set
            {
                validateDitheringLevel(value);
                ditheringLevel = value;
            }
        }

        void validateThreads(byte value)
        {
            if(value > Environment.ProcessorCount)
                throw new ArgumentOutOfRangeException();
        }

        public byte TransparencyThreshold { get; set; }

        public bool MultiplyByAlphaChannel { get; set; }

        public bool Interlace { get; set; }

        public bool Optimize { get; set; }

        public bool Quiet { get; set; }

        public bool Zopfli { get; set; }

        public bool LibDeflater { get; set; }

        private void validateCompression(byte value)
        {
            if (value > 7 || value < 1)
            {
                throw new ArgumentOutOfRangeException();
            }
        }

        private void validateDitheringLevel(byte value)
        {
            if (value > 8)
            {
                throw new ArgumentOutOfRangeException();
            }
        }
    }
}